#pragma once

#include <memory>
#include <cmath>
#include <derivative_engine.h>

namespace term
{
	struct exp_term final : cached
	{
		value_type miss(double point) override
		{
			double result = std::exp(point);
			return { result, result };
		}
	};

	struct log_term final : cached
	{
		value_type miss(double point) override
		{
			return { std::log(point), 1 / point };
		}
	};

	struct sin_term final : cached
	{
		value_type miss(double point) override
		{
			return { std::sin(point), std::cos(point) };
		}
	};

	struct cos_term final : cached
	{
		value_type miss(double point) override
		{
			return { std::cos(point), -std::sin(point) };
		}
	};

	struct tan_term final : cached
	{
		value_type miss(double point) override
		{
			double result = std::tan(point);
			return { result, 1 + result * result };
		}
	};

	template<class comp_term>
	struct composite final : variable
	{
		composite() :
			variable(new comp_term) {}
		composite(const variable& var) :
			variable(composite()(var)) {}
	};

	using exp = composite<exp_term>;
	using log = composite<log_term>;
	using sin = composite<sin_term>;
	using cos = composite<cos_term>;
	using tan = composite<tan_term>;
}