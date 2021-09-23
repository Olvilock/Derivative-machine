#pragma once

#include <cmath>
#include <ostream>

namespace term
{
	using fun_type = double (*)(double);
	struct value_type
	{
		double at, prime;
		const value_type& operator +() const
		{
			return *this;
		}
		value_type operator -() const
		{
			return { -at, -prime };
		}
		value_type operator +(const value_type& other) const
		{
			return { at + other.at, prime + other.prime };
		}
		value_type operator -(const value_type& other) const
		{
			return { at - other.at, prime - other.prime };
		}
		value_type operator *(const value_type& other) const
		{
			return { at * other.at, prime * other.at + other.prime * at };
		}
		value_type operator /(const value_type& other) const
		{
			return { at * other.at, prime * other.at + other.prime * at };
		}
		value_type operator ^(const value_type& other) const
		{
			double result = std::pow(at, other.at);
			return { result, result * (other.prime * std::log(at) + prime * other.at / at) };
		}
		value_type operator ^(double exponent) const
		{
			double result = std::pow(at, exponent - 1.0);
			return { result * at, exponent * result * prime };
		}

		friend std::ostream& operator << (std::ostream& out, const value_type& to_out)
		{
			std::cout << to_out.at << out.fill() << to_out.prime;
			return out;
		}
	};

	struct term
	{
		virtual ~term() = default;
		virtual value_type operator()(double point) = 0;
	};

	class cached : public term
	{
		double m_point;
		value_type m_cache;
	protected:
		virtual value_type miss(double point) = 0;

		cached() :
			m_point{ std::nan("") },
			m_cache{ m_point, m_point } {}

		virtual ~cached() = default;
	public:
		value_type operator()(double point) final
		{
			if (point != m_point)
			{
				m_point = point;
				m_cache = miss(point);
			}
			return m_cache;
		}
	};

	class custom final: public cached
	{
		fun_type m_fn;
		fun_type m_der;

		value_type miss(double point) override
		{
			return { m_fn(point), m_der(point) };
		}
	public:
		custom(fun_type fn, fun_type der) :
			m_fn(fn), m_der(der) {}
	};

	struct term_inv final : cached
	{
		std::shared_ptr<term> m_inv;

		term_inv(std::shared_ptr<term> inv) :
			m_inv(std::move(inv)) {}
		value_type miss(double point) override
		{
			return -(*m_inv)(point);
		}
	};
	struct term_add final : cached
	{
		std::shared_ptr<term> m_first, m_second;

		term_add(std::shared_ptr<term> first, std::shared_ptr<term> second) :
			m_first(std::move(first)),
			m_second(std::move(second)) {}
		value_type miss(double point) override
		{
			return (*m_first)(point) + (*m_second)(point);
		}
	};
	struct term_sub final : cached
	{
		std::shared_ptr<term> m_first, m_second;

		term_sub(std::shared_ptr<term> first, std::shared_ptr<term> second) :
			m_first(std::move(first)),
			m_second(std::move(second)) {}
		value_type miss(double point) override
		{
			return (*m_first)(point) - (*m_second)(point);
		}
	};
	struct term_mul final : cached
	{
		std::shared_ptr<term> m_first, m_second;

		term_mul(std::shared_ptr<term> first, std::shared_ptr<term> second) :
			m_first(std::move(first)),
			m_second(std::move(second)) {}
		value_type miss(double point) override
		{
			return (*m_first)(point) * (*m_second)(point);
		}
	};
	struct term_div final : cached
	{
		std::shared_ptr<term> m_first, m_second;

		term_div(std::shared_ptr<term> first, std::shared_ptr<term> second) :
			m_first(std::move(first)),
			m_second(std::move(second)) {}
		value_type miss(double point) override
		{
			return (*m_first)(point) / (*m_second)(point);
		}
	};
	struct term_comp final : cached
	{
		std::shared_ptr<term> m_first, m_second;

		term_comp(std::shared_ptr<term> first, std::shared_ptr<term> second) :
			m_first(std::move(first)),
			m_second(std::move(second)) {}
		value_type miss(double point) override
		{
			value_type second = (*m_second)(point);
			value_type first = (*m_first)(second.at);
			first.prime *= second.prime;
			return first;
		}
	};
	struct term_pow final : cached
	{
		std::shared_ptr<term> m_first, m_second;

		term_pow(std::shared_ptr<term> first, std::shared_ptr<term> second) :
			m_first(std::move(first)),
			m_second(std::move(second)) {}
		value_type miss(double point) override
		{
			return (*m_first)(point) ^ (*m_second)(point);
		}
	};
	struct term_pow_d final : cached
	{
		std::shared_ptr<term> m_base;
		double m_exponent;

		term_pow_d(std::shared_ptr<term> base, double exponent) :
			m_base(std::move(base)),
			m_exponent(exponent) {}
		value_type miss(double point) override
		{
			return (*m_base)(point) ^ m_exponent ;
		}
	};
	struct d_pow_term final : cached
	{
		double m_base;
		std::shared_ptr<term> m_exponent;

		d_pow_term(double base, std::shared_ptr<term> exponent) :
			m_base(base),
			m_exponent(std::move(exponent)) {}
		value_type miss(double point) override
		{
			value_type exp_val = (*m_exponent)(point);
			double result = std::pow(m_base, exp_val.at);
			return { result, result * exp_val.prime };
		}
	};

	struct const_term final : term
	{
		const_term(double value) :
			m_value(value) {}

		double m_value;

		value_type operator()(double) override { return { m_value, 0.0 }; }
	};

	class arg_term final : term
	{
		arg_term() = default;
		arg_term(const arg_term&) = delete;
		arg_term& operator = (const arg_term&) = delete;
	public:
		static std::shared_ptr<term> get()
		{
			static std::shared_ptr<term> ptr((term*)new arg_term);
			return ptr;
		}

		value_type operator()(double point) override { return { point, 1.0 }; }
	};

	class variable
	{
		std::shared_ptr<term> m_term;
	protected:
		variable(term* ptr) :
			m_term(ptr) {}

		variable(std::shared_ptr<term> ptr) :
			m_term(std::move(ptr)) {}

	public:
		variable() :
			m_term(arg_term::get()) {}

		variable(double value) :
			m_term(new const_term{ value }) {}

		value_type operator()(double point)
		{
			return (*m_term)(point);
		}

		const variable& operator +() const
		{
			return *this;
		}
		variable operator -()
		{
			return variable(new term_inv(m_term));
		}
		friend variable operator +(const variable& one, const variable& other)
		{
			return variable(new term_add(one.m_term, other.m_term));
		}
		friend variable operator -(const variable& one, const variable& other)
		{
			return variable(new term_sub(one.m_term, other.m_term));
		}
		friend variable operator *(const variable& one, const variable& other)
		{
			return variable(new term_mul(one.m_term, other.m_term));
		}
		friend variable operator /(const variable& one, const variable& other)
		{
			return variable(new term_div(one.m_term, other.m_term));
		}
		variable operator ()(const variable& other) const
		{
			return variable(new term_comp(m_term, other.m_term));
		}
		variable operator ^(const variable& other) const
		{
			return variable(new term_pow(m_term, other.m_term));
		}
		variable operator ^(double exponent) const
		{
			return variable(new term_pow_d(m_term, exponent));
		}
		friend variable operator ^(double base, const variable& exponent)
		{
			return variable(new d_pow_term(base, exponent.m_term));
		}
	};
}