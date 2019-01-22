//#undef NORMALMAP
#include "Uniforms.glsl"
#include "Samplers.glsl"
#include "Transform.glsl"
#include "ScreenPos.glsl"
#include "Lighting.glsl"
#include "Fog.glsl"


#ifdef NORMALMAP
	varying vec4 vTexCoord;
    varying vec4 vTangent;
	#ifdef MODELNORMAL
		varying vec2 vModelTexCoord;
	#endif
#else
    varying vec2 vTexCoord;
#endif

//varying vec2 vTexCoord;
varying vec3 vDetailTexCoord;
varying vec3 vNormal;
varying vec4 vWorldPos;


#ifdef PERPIXEL
    #ifdef SHADOW
        #ifndef GL_ES
            varying vec4 vShadowPos[NUMCASCADES];
        #else
            varying highp vec4 vShadowPos[NUMCASCADES];
        #endif
    #endif
    #ifdef SPOTLIGHT
        varying vec4 vSpotPos;
    #endif
    #ifdef POINTLIGHT
        varying vec3 vCubeMaskVec;
    #endif
#else
    varying vec3 vVertexLight;
    varying vec4 vScreenPos;
    #ifdef ENVCUBEMAP
        varying vec3 vReflectionVec;
    #endif
    #if defined(LIGHTMAP) || defined(AO)
        varying vec2 vTexCoord2;
    #endif
#endif

uniform sampler2D sWeightMap0;
uniform sampler2D sWeightMap1;
uniform sampler2DArray sDetailMap2;

#ifdef EDITING
	uniform vec4 cCursor;

	float calcCursor(vec4 cursor, vec4 worldpos)
	{
		// cursor format:
		// x,y : position
		// z   : radius
		// w   : hardness
		float dx=cursor.x-worldpos.x;
		float dy=cursor.y-worldpos.z;
		float len=sqrt(dx*dx+dy*dy);
		float i=(len-cursor.z)/(cursor.w*cursor.z-cursor.z);
		i=clamp(i,0.0,1.0);
		return i;
	};
#endif

#ifdef NORMALMAP
	uniform sampler2DArray sNormal3;
	#ifdef MODELNORMAL
		uniform sampler2D sModelNormal4;
	#endif
#endif

#ifdef USEMASKTEXTURE
	uniform sampler2D sMask4;
#endif

uniform vec2 cTilingFactors;
uniform vec4 cTopTilingScale;
uniform vec4 cSideTilingScale;

#ifdef MODELNORMAL
	vec3 combineNormals(vec3 n1, vec3 n2)
	{
		vec3 t=n1*vec3(2,2,2)+vec3(-1,-1,0);
		vec3 u=n2*vec3(-2,-2,2)+vec3(1,1,-1);
		vec3 r=t*dot(t,u)/t.z-u;
		return r;
	}
#endif

//uniform vec3 cDetailTiling;

void VS()
{
    mat4 modelMatrix = iModelMatrix;
    vec3 worldPos = GetWorldPos(modelMatrix);
    gl_Position = GetClipPos(worldPos);
    vNormal = GetWorldNormal(modelMatrix);
    vWorldPos = vec4(worldPos, GetDepth(gl_Position));

	#ifdef NORMALMAP
        vec4 tangent = GetWorldTangent(modelMatrix);
        vec3 bitangent = cross(tangent.xyz, vNormal) * tangent.w;
        //vTexCoord = vec4(GetTexCoord(iTexCoord), bitangent.xy);
		vTexCoord=vec4(GetTexCoord(worldPos.xz*vec2(1.0/cTilingFactors.x, 1.0/cTilingFactors.y)), bitangent.xy);
        vTangent = vec4(tangent.xyz, bitangent.z);
		#ifdef MODELNORMAL
			vModelTexCoord=GetTexCoord(iTexCoord);
		#endif
    #else
        vTexCoord = GetTexCoord(worldPos.xz*vec2(1.0/cTilingFactors.x, 1.0/cTilingFactors.y));
    #endif
    vDetailTexCoord = worldPos.xyz * 0.8;

    #ifdef PERPIXEL
        // Per-pixel forward lighting
        vec4 projWorldPos = vec4(worldPos, 1.0);

        #ifdef SHADOW
            // Shadow projection: transform from world space to shadow space
            for (int i = 0; i < NUMCASCADES; i++)
                vShadowPos[i] = GetShadowPos(i, vNormal, projWorldPos);
        #endif

        #ifdef SPOTLIGHT
            // Spotlight projection: transform from world space to projector texture coordinates
            vSpotPos = projWorldPos * cLightMatrices[0];
        #endif

        #ifdef POINTLIGHT
            vCubeMaskVec = (worldPos - cLightPos.xyz) * mat3(cLightMatrices[0][0].xyz, cLightMatrices[0][1].xyz, cLightMatrices[0][2].xyz);
        #endif
    #else
        // Ambient & per-vertex lighting
        #if defined(LIGHTMAP) || defined(AO)
            // If using lightmap, disregard zone ambient light
            // If using AO, calculate ambient in the PS
            vVertexLight = vec3(0.0, 0.0, 0.0);
            vTexCoord2 = iTexCoord1;
        #else
            vVertexLight = GetAmbient(GetZonePos(worldPos));
        #endif

        #ifdef NUMVERTEXLIGHTS
            for (int i = 0; i < NUMVERTEXLIGHTS; ++i)
                vVertexLight += GetVertexLight(i, worldPos, vNormal) * cVertexLights[i * 3].rgb;
        #endif

        vScreenPos = GetScreenPos(gl_Position);

        #ifdef ENVCUBEMAP
            vReflectionVec = worldPos - cCameraPos;
        #endif
    #endif
}

void PS()
{
    // Get material diffuse albedo
    vec4 weights0 = texture(sWeightMap0, vTexCoord.xy).rgba;
	vec4 weights1 = texture(sWeightMap1, vTexCoord.xy).rgba;

	#ifdef USEMASKTEXTURE
		vec3 mask=texture(sMask4, vTexCoord.xy).rgb;
	#endif

	//vec3 n1=vec3(0.5,0,0.86602540378443864676372317075294);
	//vec3 n3=vec3(0.5,0,-0.86602540378443864676372317075294);

	vec3 nrm = normalize(vNormal);
	//vec4 blending;
	vec3 blending;
	blending.y=abs(nrm.y);
	//blending.x=clamp((abs(dot(nrm, n1))-0.5),0,1)*2;
	//blending.z=clamp((abs(nrm.x)-0.5),0,1)*2;
	//blending.w=clamp((abs(dot(nrm,n3))-0.5),0,1)*2;
	//float b=blending.x+blending.y+blending.z+blending.w;
	blending.x=abs(nrm.x);
	blending.z=abs(nrm.z);
	float b=blending.x+blending.y+blending.z;
	blending=blending/b;

	//vec2 scale=vec2(1.1547,1);
	vec4 top1=texture(sDetailMap2, vec3(vDetailTexCoord.xz*cTopTilingScale.x,0));
	vec4 top2=texture(sDetailMap2, vec3(vDetailTexCoord.xz*cTopTilingScale.y,1));
	vec4 top3=texture(sDetailMap2, vec3(vDetailTexCoord.xz*cTopTilingScale.z,2));
	vec4 top4=texture(sDetailMap2, vec3(vDetailTexCoord.xz*cTopTilingScale.w,3));

	float ma=max(top1.a+weights0.r, max(top2.a+weights0.g, max(top3.a+weights0.b, top4.a+weights0.a)))-0.2;
	vec4 b1;
	b1.x=max(0, top1.a+weights0.r-ma);
	b1.y=max(0, top2.a+weights0.g-ma);
	b1.z=max(0, top3.a+weights0.b-ma);
	b1.w=max(0, top4.a+weights0.a-ma);
	float bsum1=b1.x+b1.y+b1.z+b1.w;

	vec4 top=((top1*b1.x+top2*b1.y+top3*b1.z+top4*b1.w)/bsum1);

	vec4 side1=texture(sDetailMap2, vec3(vDetailTexCoord.xy*cSideTilingScale.x,4));
	vec4 side2=texture(sDetailMap2, vec3(vDetailTexCoord.xy*cSideTilingScale.y,5));
	vec4 side3=texture(sDetailMap2, vec3(vDetailTexCoord.xy*cSideTilingScale.z,6));
	vec4 side4=texture(sDetailMap2, vec3(vDetailTexCoord.xy*cSideTilingScale.w,7));

	ma=max(side1.a+weights1.r, max(side2.a+weights1.g, max(side3.a+weights1.b, side4.a+weights1.a)))-0.2;
	vec4 b2;
	b2.x=max(0, side1.a+weights1.r-ma);
	b2.y=max(0, side2.a+weights1.g-ma);
	b2.z=max(0, side3.a+weights1.b-ma);
	b2.w=max(0, side4.a+weights1.a-ma);
	float bsum2=b2.x+b2.y+b2.z+b2.w;

	vec4 sideZ=((side1*b2.x+side2*b2.y+side3*b2.z+side4*b2.w)/bsum2);

	side1=texture(sDetailMap2, vec3(vDetailTexCoord.zy*cSideTilingScale.x,4));
	side2=texture(sDetailMap2, vec3(vDetailTexCoord.zy*cSideTilingScale.y,5));
	side3=texture(sDetailMap2, vec3(vDetailTexCoord.zy*cSideTilingScale.z,6));
	side4=texture(sDetailMap2, vec3(vDetailTexCoord.zy*cSideTilingScale.w,7));

	ma=max(side1.a+weights1.r, max(side2.a+weights1.g, max(side3.a+weights1.b, side4.a+weights1.a)))-0.2;
	vec4 b3;
	b3.x=max(0, side1.a+weights1.r-ma);
	b3.y=max(0, side2.a+weights1.g-ma);
	b3.z=max(0, side3.a+weights1.b-ma);
	b3.w=max(0, side4.a+weights1.a-ma);
	float bsum3=b3.x+b3.y+b3.z+b3.w;

	vec4 sideX=((side1*b3.x+side2*b3.y+side3*b3.z+side4*b3.w)/bsum3);


	vec4 diffColor=top*blending.y + sideX*blending.x + sideZ*blending.z;// + sideA*blending.w;

	#ifdef USEMASKTEXTURE
		diffColor.r=mix(1.0, diffColor.r, mask.r);
		diffColor.g=mix(1.0, diffColor.g, mask.g);
		diffColor.b=mix(1.0, diffColor.b, mask.b);
	#endif

	//float grad=clamp(vWorldPos.y*0.125,0.5,1);
	//diffColor*=grad;
	//diffColor=vec4(1,1,1,1);

    // Get material specular albedo
    vec3 specColor = cMatSpecColor.rgb;

    // Get normal
	#ifdef NORMALMAP
        mat3 tbn = mat3(vTangent.xyz, vec3(vTexCoord.zw, vTangent.w), vNormal);

		top1=texture(sNormal3, vec3(vDetailTexCoord.xz*cTopTilingScale.x,0));
		top2=texture(sNormal3, vec3(vDetailTexCoord.xz*cTopTilingScale.y,1));
		top3=texture(sNormal3, vec3(vDetailTexCoord.xz*cTopTilingScale.z,2));
		top4=texture(sNormal3, vec3(vDetailTexCoord.xz*cTopTilingScale.w,3));
		top=((top1*b1.x+top2*b1.y+top3*b1.z+top4*b1.w)/bsum1);

		side1=texture(sNormal3, vec3(vDetailTexCoord.xy*cSideTilingScale.x,4));
		side2=texture(sNormal3, vec3(vDetailTexCoord.xy*cSideTilingScale.y,5));
		side3=texture(sNormal3, vec3(vDetailTexCoord.xy*cSideTilingScale.z,6));
		side4=texture(sNormal3, vec3(vDetailTexCoord.xy*cSideTilingScale.w,7));
		sideZ=((side1*b2.x+side2*b2.y+side3*b2.z+side4*b2.w)/bsum2);

		side1=texture(sNormal3, vec3(vDetailTexCoord.zy*cSideTilingScale.x,4));
		side2=texture(sNormal3, vec3(vDetailTexCoord.zy*cSideTilingScale.y,5));
		side3=texture(sNormal3, vec3(vDetailTexCoord.zy*cSideTilingScale.z,6));
		side4=texture(sNormal3, vec3(vDetailTexCoord.zy*cSideTilingScale.w,7));
		sideX=((side1*b3.x+side2*b3.y+side3*b3.z+side4*b3.w)/bsum3);

		vec3 texnormal=normalize((top*blending.y + sideX*blending.x + sideZ*blending.z).rgb * 2.0 - 1.0);


		#ifdef MODELNORMAL
			texnormal=texnormal*0.5+0.5;
			vec4 modelnormalsample=texture(sModelNormal4, vModelTexCoord.xy);
			vec3 modelnormal=modelnormalsample.rgb;
			vec3 normal=(combineNormals(modelnormal, texnormal));
			//vec3 normal=modelnormal*2-1;
		#else
			vec3 normal=texnormal;
		#endif

		normal=normalize(tbn*normal);

    #else
        vec3 normal = normalize(vNormal);
    #endif


    // Get fog factor
    #ifdef HEIGHTFOG
        float fogFactor = GetHeightFogFactor(vWorldPos.w, vWorldPos.y);
    #else
        float fogFactor = GetFogFactor(vWorldPos.w);
    #endif

    #if defined(PERPIXEL)
        // Per-pixel forward lighting
        vec3 lightColor;
        vec3 lightDir;
        vec3 finalColor;

        float diff = GetDiffuse(normal, vWorldPos.xyz, lightDir);


        #ifdef SHADOW
            diff *= GetShadow(vShadowPos, vWorldPos.w);
        #endif

        #if defined(SPOTLIGHT)
            lightColor = vSpotPos.w > 0.0 ? texture2DProj(sLightSpotMap, vSpotPos).rgb * cLightColor.rgb : vec3(0.0, 0.0, 0.0);
        #elif defined(CUBEMASK)
            lightColor = textureCube(sLightCubeMap, vCubeMaskVec).rgb * cLightColor.rgb;
        #else
            lightColor = cLightColor.rgb;
        #endif

        #ifdef SPECULAR
            float spec = GetSpecular(normal, cCameraPosPS - vWorldPos.xyz, lightDir, cMatSpecColor.a);
            finalColor = diff * lightColor * (diffColor.rgb + spec * specColor * cLightColor.a);
        #else
            finalColor = diff * lightColor * diffColor.rgb;
        #endif

        #ifdef AMBIENT
            finalColor += cAmbientColor.rgb * diffColor.rgb;
            finalColor += cMatEmissiveColor;
            gl_FragColor = vec4(GetFog(finalColor, fogFactor), diffColor.a);
        #else
            gl_FragColor = vec4(GetLitFog(finalColor, fogFactor), diffColor.a);
        #endif
    #elif defined(PREPASS)
        // Fill light pre-pass G-Buffer
        float specPower = cMatSpecColor.a / 255.0;

        gl_FragData[0] = vec4(normal * 0.5 + 0.5, specPower);
        gl_FragData[1] = vec4(EncodeDepth(vWorldPos.w), 0.0);
    #elif defined(DEFERRED)
        // Fill deferred G-buffer
        float specIntensity = specColor.g;
        float specPower = cMatSpecColor.a / 255.0;

        vec3 finalColor = vVertexLight * diffColor.rgb;
        #ifdef AO
            // If using AO, the vertex light ambient is black, calculate occluded ambient here
            finalColor += texture2D(sEmissiveMap, vTexCoord2).rgb * cAmbientColor.rgb * diffColor.rgb;
        #endif

        #ifdef ENVCUBEMAP
            finalColor += cMatEnvMapColor * textureCube(sEnvCubeMap, reflect(vReflectionVec, normal)).rgb;
        #endif
        #ifdef LIGHTMAP
            finalColor += texture2D(sEmissiveMap, vTexCoord2).rgb * diffColor.rgb;
        #endif
        #ifdef EMISSIVEMAP
            finalColor += cMatEmissiveColor * texture2D(sEmissiveMap, vTexCoord.xy).rgb;
        #else
            finalColor += cMatEmissiveColor;
        #endif

        gl_FragData[0] = vec4(GetFog(finalColor, fogFactor), 1.0);
        gl_FragData[1] = fogFactor * vec4(diffColor.rgb, specIntensity);
        gl_FragData[2] = vec4(normal * 0.5 + 0.5, specPower);
        gl_FragData[3] = vec4(EncodeDepth(vWorldPos.w), 0.0);
    #else
        // Ambient & per-vertex lighting
        vec3 finalColor = vVertexLight * diffColor.rgb;
        #ifdef AO
            // If using AO, the vertex light ambient is black, calculate occluded ambient here
            finalColor += texture2D(sEmissiveMap, vTexCoord2).rgb * cAmbientColor.rgb * diffColor.rgb;
        #endif

        #ifdef MATERIAL
            // Add light pre-pass accumulation result
            // Lights are accumulated at half intensity. Bring back to full intensity now
            vec4 lightInput = 2.0 * texture2DProj(sLightBuffer, vScreenPos);
            vec3 lightSpecColor = lightInput.a * lightInput.rgb / max(GetIntensity(lightInput.rgb), 0.001);

            finalColor += lightInput.rgb * diffColor.rgb + lightSpecColor * specColor;
        #endif

        #ifdef ENVCUBEMAP
            finalColor += cMatEnvMapColor * textureCube(sEnvCubeMap, reflect(vReflectionVec, normal)).rgb;
        #endif
        #ifdef LIGHTMAP
            finalColor += texture2D(sEmissiveMap, vTexCoord2).rgb * diffColor.rgb;
        #endif
        #ifdef EMISSIVEMAP
            finalColor += cMatEmissiveColor * texture2D(sEmissiveMap, vTexCoord.xy).rgb;
        #else
            finalColor += cMatEmissiveColor;
        #endif

        gl_FragColor = vec4(GetFog(finalColor, fogFactor), diffColor.a);
    #endif
	#ifdef EDITING
		float j=calcCursor(cCursor, vWorldPos);
		gl_FragColor = mix(gl_FragColor, vec4(1,1,1,1), j*0.75);
	#endif
}
