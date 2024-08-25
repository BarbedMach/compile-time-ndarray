import std;

template<typename T, std::size_t... Dims> struct ndarray;

template<typename T, std::size_t FirstDim, std::size_t... RestOfDims> struct ndarray<T, FirstDim, RestOfDims...> {
	ndarray<T, RestOfDims...> m_data[FirstDim];
	std::size_t m_dimSize{ FirstDim };
	std::array<std::size_t, sizeof...(RestOfDims)> m_restOfDimSizes{ {(RestOfDims, ...)} };

	constexpr ndarray() = default;

	constexpr ndarray(const T& value) {
		for (std::size_t i = 0; i < FirstDim; ++i) {
			m_data[i] = ndarray<T, RestOfDims...>(value);
		}
	}

	constexpr ndarray(const std::initializer_list<ndarray<T, RestOfDims...>>& list) {
		if (list.size() != FirstDim) {
			throw std::runtime_error("Initializer list size does not match the allocated dimension.");
		}
		std::size_t index = 0;
		for (const auto& elem : list) {
			m_data[index++] = elem;
		} 
	}

	template<typename Self>
	[[nodiscard]] constexpr auto&& operator[](this Self&& self, std::size_t index) noexcept {
		return std::forward_like<Self>(self.m_data[index]);
	}

	[[nodiscard]] constexpr ndarray<T, RestOfDims...> operator[](std::size_t index) && noexcept {
		return m_data[index];
	}

	[[nodiscard]] inline constexpr auto dimensions() const noexcept {
		return std::array{ FirstDim, (RestOfDims , ...) };
	}
};

template<typename T, std::size_t LastDim> struct ndarray<T, LastDim> {
	T m_data[LastDim];
	std::size_t m_dimSize{ LastDim };

	constexpr ndarray() = default;

	constexpr ndarray(const T& value) {
		for (std::size_t i = 0; i < LastDim; ++i) {
			m_data[i] = value;
		}
	}

	template<typename... Values>
	requires (sizeof...(Values) <= LastDim && (std::is_convertible_v<Values, T> && ...))
	constexpr ndarray(Values&&... values) {
		std::size_t index = 0;
		((index < sizeof...(Values) ? m_data[index++] = std::forward<Values>(values) : Values{}), ...);
		while (index < LastDim) {
			m_data[index++] = T{};
		}
	}

	template<typename Self>
	[[nodiscard]] constexpr auto&& operator[](this Self&& self, std::size_t index) noexcept {
		return std::forward_like<Self>(self.m_data[index]);
	}

	[[nodiscard]] constexpr T operator[](std::size_t index) && noexcept {
		return m_data[index];
	}

	[[nodiscard]] inline constexpr auto dimensions() const noexcept {
		return std::array{ LastDim };
	}
};

template<typename T, std::size_t FirstDim, std::size_t... RestOfDims>
struct std::formatter<ndarray<T, FirstDim, RestOfDims...>> {
	constexpr auto parse(std::format_parse_context& format_parse_context) {
		return format_parse_context.begin();
	}
	auto format(const ndarray<T, FirstDim, RestOfDims...>& ndarray, std::format_context& format_context) const {
		return format_impl(ndarray, format_context, 0);
	}

private:
	auto format_impl(const ndarray<T, FirstDim, RestOfDims...>& ndarray, std::format_context& format_context, int indent_level) const {
		auto out = std::format_to(format_context.out(), "{:>{}}[\n", "", indent_level);

		for (std::size_t i = 0; i < FirstDim; ++i) {
			if (i > 0) {
				out = std::format_to(out, ",\n");
			}
			out = std::format_to(out, "{:>{}}{}", "", indent_level + 2, ndarray.m_data[i]);
		}

		return std::format_to(out, "\n{:>{}}]", "", indent_level);
	}
};

template<typename T, std::size_t LastDim> struct std::formatter<ndarray<T, LastDim>> {
	constexpr auto parse(std::format_parse_context& format_parse_context) {
		return format_parse_context.begin();
	}

	auto format(const ndarray<T, LastDim>& ndarray, std::format_context& format_context) const {
		auto out = std::format_to(format_context.out(), "[");
		
		for (auto i = 0; i < LastDim; ++i) {
			if (i > 0) {
				out = std::format_to(out, ", ");
			}
			out = std::format_to(out, "{}", ndarray[i]);
		}

		return std::format_to(out, "]");
	}
};

template<typename T, std::size_t N> struct std::formatter<std::array<T, N>> {
	constexpr auto parse(std::format_parse_context& format_parse_context) {
		return format_parse_context.begin();
	}

	auto format(const std::array<T, N>& array, std::format_context& format_context) const {
		auto out = std::format_to(format_context.out(), "[");

		for (auto i = 0; i < N; ++i) {
			if (i > 0) {
				out = std::format_to(out, ", ");
			}
			out = std::format_to(out, "{}", array[i]);
		}

		return std::format_to(out, "]");
	}
};

auto main() -> int {
	try {
		const auto arr{ ndarray<int, 2, 5>{ ndarray<int, 5>{1, 2, 3}, ndarray<int, 5>{1, 2, 3 ,4 ,5} } };
		std::println("{}", arr);
		std::println("Current dimensions of arr: {}", arr.dimensions());
	}
	catch (const std::runtime_error& e) {
		std::println("{}", e.what());
	}
}