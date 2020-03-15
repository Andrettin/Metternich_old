#pragma once

#include <type_traits>

template<typename T, template<typename...> class T2>
struct is_specialization_of : std::false_type {};

template<template<typename...> class T, typename ...A>
struct is_specialization_of<T<A...>, T> : std::true_type {};

template<typename T, template<typename...> class T2>
inline constexpr bool is_specialization_of_v = is_specialization_of<T, T2>::value;

template<typename T>
struct contained_element
{
	using type = T;
};

template<typename T>
struct contained_element<std::vector<T>>
{
	using type = T;
};

template <typename T>
using contained_element_t = typename contained_element<T>::type;
