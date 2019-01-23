#pragma once
#include <vector>
#include <cmath>

// Weighted table selection

double rolld(double,double);

class WeightedTable
{
	public:
	WeightedTable(double weight, int numchoices) : table_(numchoices)
	{
		weight=std::abs(weight);
		double total=0.0;
		for(int i=0; i<numchoices; ++i)
		{
			double wt=std::pow(weight, (double)i);
			total+=wt;
			table_[i]=total;
		}
		for(int i=0; i<numchoices; ++i) table_[i] /= total;
	}

	int Choose()
	{
		int numchoices=table_.size();
		double rl=rolld(0.0, 1.0);

		for(int i=0; i<numchoices; ++i)
		{
			if(rl < table_[i])
			{
			return i;
			}
		}
		return -1;
	}

	protected:
	std::vector<double> table_;
};
