#pragma once
#include <optional>
#include "pugixml.hpp"

namespace pugi_utils {

std::optional<long> get_long_attr(const pugi::xml_node& node, const std::wstring& name);
std::optional<double> get_double_attr(const pugi::xml_node& node, const std::wstring& name);
std::wstring get_wstring_attr(const pugi::xml_node& node, const std::wstring& name);

} // namespace pugi_utils