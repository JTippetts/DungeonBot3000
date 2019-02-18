//
// Copyright (c) 2008-2017 the Urho3D project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include <Urho3D/Core/Context.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/Technique.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Texture2D.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/Geometry.h>
#include <Urho3D/Graphics/IndexBuffer.h>
#include <Urho3D/Graphics/VertexBuffer.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/UIBatch.h>
#include <Urho3D/Resource/ResourceCache.h>

#include <Urho3D/IO/Log.h>

#include "UIMesh.h"

#include <Urho3D/DebugNew.h>
//=============================================================================
//=============================================================================
UIMesh::UIMesh(Context* context)
    : UIElement(context)
    , blendMode_(BLEND_REPLACE)
{
}

UIMesh::~UIMesh()
{
}

void UIMesh::RegisterObject(Context* context)
{
    context->RegisterFactory<UIMesh>();

	URHO3D_COPY_BASE_ATTRIBUTES(UIElement);
	URHO3D_MIXED_ACCESSOR_ATTRIBUTE("Model", GetModelAttr, SetModelAttr, ResourceRef, ResourceRef(Model::GetTypeStatic()), AM_DEFAULT);
    URHO3D_MIXED_ACCESSOR_ATTRIBUTE("Material", GetMaterialAttr, SetMaterialAttr, ResourceRef, ResourceRef(Material::GetTypeStatic()), AM_DEFAULT);
}

void UIMesh::GetBatches(PODVector<UIBatch>& batches, PODVector<float>& vertexData, const IntRect& currentScissor)
{
    const IntVector2& screenPos = GetScreenPosition();
    IntVector2 intDeltaPos;
    Vector2 deltaPos;

    if (screenPos != position_)
    {
        intDeltaPos = screenPos - position_;
        deltaPos = Vector2(intDeltaPos);
    }

    for (unsigned i = 0; i < batches_.Size(); ++i)
    {
        // get batch
        UIBatch &batch     = batches_[i];
        unsigned beg       = batch.vertexStart_;
        unsigned end       = batch.vertexEnd_;
        batch.vertexStart_ = vertexData.Size();
        batch.vertexEnd_   = vertexData.Size() + (end - beg);

        vertexData.Resize(batch.vertexEnd_);

        for (unsigned j = 0; j < (end - beg)/UI_VERTEX_SIZE; ++j)
        {
            vertexData[batch.vertexStart_ + j*UI_VERTEX_SIZE + 0] = workingVertexData_[beg + j*UI_VERTEX_SIZE + 0] + deltaPos.x_;
            vertexData[batch.vertexStart_ + j*UI_VERTEX_SIZE + 1] = workingVertexData_[beg + j*UI_VERTEX_SIZE + 1] + deltaPos.y_;

            vertexData[batch.vertexStart_ + j*UI_VERTEX_SIZE + 2] = workingVertexData_[beg + j*UI_VERTEX_SIZE + 2];
            vertexData[batch.vertexStart_ + j*UI_VERTEX_SIZE + 3] = workingVertexData_[beg + j*UI_VERTEX_SIZE + 3];
            vertexData[batch.vertexStart_ + j*UI_VERTEX_SIZE + 4] = workingVertexData_[beg + j*UI_VERTEX_SIZE + 4];
            vertexData[batch.vertexStart_ + j*UI_VERTEX_SIZE + 5] = workingVertexData_[beg + j*UI_VERTEX_SIZE + 5];
        }

        // delta
        IntRect saveScissor = batch.scissor_;
        batch.scissor_.left_   += intDeltaPos.x_;
        batch.scissor_.top_    += intDeltaPos.y_;
        batch.scissor_.right_  += intDeltaPos.x_;
        batch.scissor_.bottom_ += intDeltaPos.y_;

        // store
        batches.Push(batch);

        // restore
        batch.vertexStart_ = beg;
        batch.vertexEnd_   = end;
        batch.scissor_     = saveScissor;
    }
}

#ifdef _WIN32
#pragma optimize("", off)
#endif

void UIMesh::SetModel(Model *model)
{
	model_=model;
    Geometry *geometry = model->GetGeometry(0, 0);
    VertexBuffer *vbuffer = geometry->GetVertexBuffer(0);
    IndexBuffer *ibuffer = geometry->GetIndexBuffer();
    unsigned elementMask = vbuffer->GetElementMask();
    Vector3 fullSize = model->GetBoundingBox().Size();
    Vector3 halfSize = model->GetBoundingBox().HalfSize();
    float unitScaleFactor = fullSize.x_>fullSize.y_?1.0f/fullSize.x_:1.0f/fullSize.z_;

    // minimum requirement
    assert((elementMask & (MASK_POSITION | MASK_TEXCOORD1)) == (MASK_POSITION | MASK_TEXCOORD1));

    // not happening for indices over 64k
    assert(ibuffer->GetIndexSize() == sizeof(unsigned short));

    unsigned indexCount = ibuffer->GetIndexCount();
    unsigned vertexSize = vbuffer->GetVertexSize();
    unsigned char *vertexData = (unsigned char*)vbuffer->Lock(0, vbuffer->GetVertexCount());
    unsigned short *ushortData = (unsigned short*)ibuffer->Lock(0, indexCount);

    batches_.Clear();
    vertexData_.Resize(indexCount * UI_VERTEX_SIZE);

    if (vertexData && ushortData)
    {
        const unsigned white = Color::WHITE.ToUInt();

        for (unsigned i = 0; i < indexCount; i += 3)
        {
            PODVector<float> vertArray(3 * UI_VERTEX_SIZE);
            int minx = M_MAX_INT, miny = M_MAX_INT;
            int maxx = M_MIN_INT, maxy = M_MIN_INT;

            for (unsigned j = 0; j < 3; ++j)
            {
                unsigned i0 = ushortData[i+j];

                unsigned char *dataAlign = (vertexData + i0 * vertexSize);
                unsigned col = white;

                // pos
                Vector3 v0 = *reinterpret_cast<Vector3*>(dataAlign);

                //**NOTE** the z(y) component is reversed due to the negative y-scaling (-2.0f * invScreenSize.y_ in Render() fn.)
                // - move verts to positive space and unitize
                v0.x_ = ( v0.x_ + halfSize.x_) * unitScaleFactor;
                v0.z_ = (-v0.z_ + halfSize.z_) * unitScaleFactor;

                dataAlign += sizeof(Vector3);

                // scissor
                if ((int)(v0.x_-0.5f) < minx) minx = (int)(v0.x_-0.5f);
                if ((int)(v0.x_+0.5f) > maxx) maxx = (int)(v0.x_+0.5f);
                if ((int)(v0.z_-0.5f) < miny) miny = (int)(v0.z_-0.5f);
                if ((int)(v0.z_+0.5f) > maxy) maxy = (int)(v0.z_+0.5f);

                // skip normal
                if (elementMask & MASK_NORMAL) dataAlign += sizeof(Vector3);

                // col
                if (elementMask & MASK_COLOR)
                {
                    col = *reinterpret_cast<unsigned*>(dataAlign);
                    dataAlign += sizeof(unsigned);
                }

                // uv
                const Vector2 &uv = *reinterpret_cast<Vector2*>(dataAlign);

                // copy triangle data
                vertArray[j*UI_VERTEX_SIZE+0] = v0.x_;
                vertArray[j*UI_VERTEX_SIZE+1] = v0.z_;
                vertArray[j*UI_VERTEX_SIZE+2] = 0.0f;
                ((unsigned&)vertArray[j*UI_VERTEX_SIZE+3]) = col;
                vertArray[j*UI_VERTEX_SIZE+4] = uv.x_;
                vertArray[j*UI_VERTEX_SIZE+5] = uv.y_;
            }

            IntRect scissor(minx, miny, maxx, maxy);
            UIBatch batch(this, blendMode_, scissor, texture_, &vertArray);

            // set start/end
            batch.vertexStart_ = i*UI_VERTEX_SIZE;
            batch.vertexEnd_   = i*UI_VERTEX_SIZE + 3*UI_VERTEX_SIZE;
            batches_.Push(batch);

            // check winding
            Vector3 v0(vertArray[0], vertArray[1], 0.0f);
            Vector3 v1(vertArray[UI_VERTEX_SIZE], vertArray[UI_VERTEX_SIZE+1], 0.0f);
            Vector3 v2(vertArray[2*UI_VERTEX_SIZE], vertArray[2*UI_VERTEX_SIZE+1], 0.0f);
            Vector3 e0 = (v1-v0);
            Vector3 e1 = (v2-v0);
            Vector3 cp = e0.CrossProduct(e1);

            // copy vertarray, change winding if needed
            if (cp.z_ < 0.0f)
            {
                memcpy(&vertexData_[batch.vertexStart_], &vertArray[0], (UI_VERTEX_SIZE)*sizeof(float));
                memcpy(&vertexData_[batch.vertexStart_ + 2*UI_VERTEX_SIZE], &vertArray[UI_VERTEX_SIZE], (UI_VERTEX_SIZE) * sizeof(float));
                memcpy(&vertexData_[batch.vertexStart_ + UI_VERTEX_SIZE], &vertArray[2*UI_VERTEX_SIZE], (UI_VERTEX_SIZE) * sizeof(float));
            }
            else
            {
                memcpy(&vertexData_[batch.vertexStart_], &vertArray[0], (3*UI_VERTEX_SIZE) * sizeof(float));
            }
        }

        vbuffer->Unlock();
        ibuffer->Unlock();

        // copy working data
        workingVertexData_.Resize(vertexData_.Size());
        memcpy(&workingVertexData_[0], &vertexData_[0], vertexData_.Size() * sizeof(float));

        // consolidate all batches into a single batch
        UIBatch batch(this, blendMode_, IntRect(0, 0, 1, 1), texture_, &workingVertexData_);
        unsigned beg = batches_[0].vertexStart_;
        unsigned end = batches_[batches_.Size() - 1].vertexEnd_;
        batch.vertexStart_ = beg;
        batch.vertexEnd_ = end;

        batches_.Clear();
        batches_.Push(batch);

        UpdateScissors();

        // set position/size if already set - call either fn, both fns adjust both
        if (position_.x_ != 0 || position_.y_ != 0 || GetSize().x_ > 1 || GetSize().y_ > 1.0f)
        {
            SetPosition(position_);
        }
    }
}

#ifdef _WIN32
#pragma optimize("", on)
#endif

void UIMesh::SetMaterial(Material *material)
{
    //ResourceCache* cache = GetSubsystem<ResourceCache>();
    //material_ = cache->GetResource<Material>(matrialFilename);
	material_ = material;

    SetCustomShader(true);
}

Model *UIMesh::GetModel()
{
	return model_;
}

Material *UIMesh::GetMaterial()
{
	return material_;
}

void UIMesh::SetModelAttr(const ResourceRef& value)
{
    auto* cache = GetSubsystem<ResourceCache>();
    SetModel(cache->GetResource<Model>(value.name_));
	SetCustomShader(true);
}

void UIMesh::SetMaterialAttr(const ResourceRef& value)
{
    auto* cache = GetSubsystem<ResourceCache>();
    SetMaterial(cache->GetResource<Material>(value.name_));
	SetCustomShader(true);
}

ResourceRef UIMesh::GetModelAttr() const
{
    return GetResourceRef(model_, Model::GetTypeStatic());
}

ResourceRef UIMesh::GetMaterialAttr() const
{
	return GetResourceRef(material_, Material::GetTypeStatic());
}

void UIMesh::Render(Graphics* graphics, const UIBatch& batch)
{
    // update material shader
    UpdateMaterialShader(graphics);

    // rest of the settings - straight copy from UI
    float uiScale = GetSubsystem<UI>()->GetScale();
    IntVector2 viewSize = graphics->GetViewport().Size();
    Vector2 invScreenSize(1.0f / (float)viewSize.x_, 1.0f / (float)viewSize.y_);
    Vector2 scale(2.0f * invScreenSize.x_, -2.0f * invScreenSize.y_);
    Vector2 offset(-1.0f, 1.0f);

    Matrix4 projection(Matrix4::IDENTITY);
    projection.m00_ = scale.x_ * uiScale;
    projection.m03_ = offset.x_;
    projection.m11_ = scale.y_ * uiScale;
    projection.m13_ = offset.y_;
    projection.m22_ = 1.0f;
    projection.m23_ = 0.0f;
    projection.m33_ = 1.0f;

    if (graphics->NeedParameterUpdate(SP_OBJECT, this))
    {
        graphics->SetShaderParameter(VSP_MODEL, Matrix3x4::IDENTITY);
    }

    if (graphics->NeedParameterUpdate(SP_CAMERA, this))
    {
        graphics->SetShaderParameter(VSP_VIEWPROJ, projection);
    }

    float elapsedTime = GetSubsystem<Time>()->GetElapsedTime();
    graphics->SetShaderParameter(VSP_ELAPSEDTIME, elapsedTime);
    graphics->SetShaderParameter(PSP_ELAPSEDTIME, elapsedTime);

    IntRect scissor = batch.scissor_;
    scissor.left_   = (int)(scissor.left_ * uiScale);
    scissor.top_    = (int)(scissor.top_ * uiScale);
    scissor.right_  = (int)(scissor.right_ * uiScale);
    scissor.bottom_ = (int)(scissor.bottom_ * uiScale);

    graphics->SetBlendMode(batch.blendMode_);
    graphics->SetScissorTest(true, scissor);

    // draw
    graphics->Draw(TRIANGLE_LIST, batch.vertexStart_ / UI_VERTEX_SIZE, (batch.vertexEnd_ - batch.vertexStart_) / UI_VERTEX_SIZE);
}

void UIMesh::UpdateMaterialShader(Graphics* graphics)
{
    if (material_)
    {
        //**NOTE** if you have more than one technique or pass then you'll need to add
        // code here - although you shouldn't need anymore than a single pass for UI, however.
        Technique *technique = material_->GetTechnique(0);
        PODVector<Pass*> passes = technique->GetPasses();
        Pass* pass = 0;

        for (unsigned i = 0; i < passes.Size(); ++i)
        {
            pass = passes[i];
            if (pass != 0)
            {
                break;
            }
        }

        ShaderVariation *vs = graphics->GetShader(VS, pass->GetVertexShader(), material_->GetVertexShaderDefines());
        ShaderVariation *ps = graphics->GetShader(PS, pass->GetPixelShader(), material_->GetPixelShaderDefines());

        graphics->SetShaders(vs, ps);

        if (graphics->NeedParameterUpdate(SP_MATERIAL, reinterpret_cast<const void*>(material_->GetShaderParameterHash())))
        {
            const HashMap<StringHash, MaterialShaderParameter>& parameters = material_->GetShaderParameters();
            for (HashMap<StringHash, MaterialShaderParameter>::ConstIterator i = parameters.Begin(); i != parameters.End(); ++i)
                graphics->SetShaderParameter(i->first_, i->second_.value_);
        }

        const HashMap<TextureUnit, SharedPtr<Texture> >& textures = material_->GetTextures();
        for (HashMap<TextureUnit, SharedPtr<Texture> >::ConstIterator i = textures.Begin(); i != textures.End(); ++i)
        {
            if (graphics->HasTextureUnit(i->first_))
                graphics->SetTexture(i->first_, i->second_.Get());
        }
    }
}

void UIMesh::SetSize(const IntVector2& size)
{
    UIElement::SetSize(size);
    Vector2 pos(position_);
    bool adjustPos = (pos.x_ > 0.0f || pos.y_ > 0.0f);

    for (unsigned i = 0; i < vertexData_.Size(); i += UI_VERTEX_SIZE)
    {
        if (adjustPos)
        {
            workingVertexData_[i+0] = vertexData_[i+0] * size.x_ + pos.x_;
            workingVertexData_[i+1] = vertexData_[i+1] * size.y_ + pos.y_;
        }
        else
        {
            workingVertexData_[i+0] = vertexData_[i+0] * size.x_;
            workingVertexData_[i+1] = vertexData_[i+1] * size.y_;
        }
    }

    UpdateScissors();
}

void UIMesh::SetSize(int width, int height)
{
    SetSize(IntVector2(width, height));
}

void UIMesh::SetPosition(const IntVector2& position)
{
    UIElement::SetPosition(position);
    Vector2 size(GetSize());
    Vector2 pos(position);
    bool adjustSize = (size.x_ > 1.0f || size.y_ > 1.0f);

    for (unsigned i = 0; i < vertexData_.Size(); i += UI_VERTEX_SIZE)
    {
        if (adjustSize)
        {
            workingVertexData_[i+0] = vertexData_[i+0] * size.x_ + pos.x_;
            workingVertexData_[i+1] = vertexData_[i+1] * size.y_ + pos.y_;
        }
        else
        {
            workingVertexData_[i+0] = vertexData_[i+0] + pos.x_;
            workingVertexData_[i+1] = vertexData_[i+1] + pos.y_;
        }
    }

    UpdateScissors();
}

void UIMesh::SetPosition(int x, int y)
{
    SetPosition(IntVector2(x, y));
}

void UIMesh::SetBlendMode(BlendMode blendMode)
{
    if (blendMode_ != blendMode)
    {
        blendMode_ = blendMode;

        if (batches_.Size() > 0)
        {
            UpdateBlendMode();
        }
    }
}

void UIMesh::UpdateScissors()
{
    for (unsigned i = 0; i < batches_.Size(); ++i)
    {
        int minx = M_MAX_INT, miny = M_MAX_INT;
        int maxx = M_MIN_INT, maxy = M_MIN_INT;

        for (unsigned j = batches_[i].vertexStart_; j < batches_[i].vertexEnd_; j += UI_VERTEX_SIZE)
        {
            Vector2 v0(workingVertexData_[j], workingVertexData_[j + 1]);

            if ((int)(v0.x_-0.5f) < minx) minx = (int)(v0.x_-0.5f);
            if ((int)(v0.x_+0.5f) > maxx) maxx = (int)(v0.x_+0.5f);
            if ((int)(v0.y_-0.5f) < miny) miny = (int)(v0.y_-0.5f);
            if ((int)(v0.y_+0.5f) > maxy) maxy = (int)(v0.y_+0.5f);
        }

        batches_[i].scissor_ = IntRect(minx, miny, maxx, maxy);
    }
}

void UIMesh::UpdateBlendMode()
{
    for (unsigned i = 0; i < batches_.Size(); ++i)
    {
        batches_[i].blendMode_ = blendMode_;
    }
}

