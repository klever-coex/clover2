#include <clover2_http_plugins/utils/msg_json.hpp>

#include <rosidl_typesupport_introspection_cpp/field_types.hpp>

#include <cstdint>
#include <string>
#include <vector>
#include <stack>

namespace clover2_http_plugins::utils::msg_json::detail {

namespace {

const void* member_ptr(
    const rosidl_typesupport_introspection_cpp::MessageMember& m,
    const void* msg) {
    return reinterpret_cast<const char*>(msg) + m.offset_;
}

void* member_ptr(const rosidl_typesupport_introspection_cpp::MessageMember& m,
                 void* msg) {
    return reinterpret_cast<char*>(msg) + m.offset_;
}

const void* field(const rosidl_typesupport_introspection_cpp::MessageMember& m,
                  const void* msg, size_t index) {
    if (!m.is_array_) {
        return member_ptr(m, msg);
    }
    return m.get_const_function(member_ptr(m, msg), index);
}

void* field(const rosidl_typesupport_introspection_cpp::MessageMember& m,
            void* msg, size_t index) {
    if (!m.is_array_) {
        return member_ptr(m, msg);
    }
    return m.get_function(member_ptr(m, msg), index);
}

size_t element_count(
    const rosidl_typesupport_introspection_cpp::MessageMember& m,
    const void* msg) {
    return m.size_function ? m.size_function(member_ptr(m, msg))
                           : m.array_size_;
}

const rosidl_typesupport_introspection_cpp::MessageMembers* nested_members(
    const rosidl_typesupport_introspection_cpp::MessageMember& m) {
    return static_cast<
        const rosidl_typesupport_introspection_cpp::MessageMembers*>(
        m.members_->data);
}

[[noreturn]] void unsupported(const std::string& what) {
    throw std::runtime_error("msg_json: unsupported " + what);
}

std::string utf16_to_utf8(const std::u16string& in) {
    std::string out;

    for (char16_t ch : in) {
        if (ch < 0x80) {
            out.push_back(static_cast<char>(ch));
        } else if (ch < 0x800) {
            out.push_back(static_cast<char>(0xC0 | (ch >> 6)));
            out.push_back(static_cast<char>(0x80 | (ch & 0x3F)));
        } else {
            out.push_back(static_cast<char>(0xE0 | (ch >> 12)));
            out.push_back(static_cast<char>(0x80 | ((ch >> 6) & 0x3F)));
            out.push_back(static_cast<char>(0x80 | (ch & 0x3F)));
        }
    }

    return out;
}

std::u16string utf8_to_utf16(const std::string& in) {
    std::u16string out;
    size_t i = 0;

    while (i < in.size()) {
        const auto c = static_cast<unsigned char>(in[i]);
        if (c < 0x80) {
            out.push_back(static_cast<char16_t>(c));
            ++i;
        } else if ((c & 0xE0) == 0xC0) {
            out.push_back(static_cast<char16_t>(
                ((c & 0x1F) << 6) |
                (static_cast<unsigned char>(in[i + 1]) & 0x3F)));
            i += 2;
        } else {
            out.push_back(static_cast<char16_t>(
                ((c & 0x0F) << 12) |
                ((static_cast<unsigned char>(in[i + 1]) & 0x3F) << 6) |
                (static_cast<unsigned char>(in[i + 2]) & 0x3F)));
            i += 3;
        }
    }

    return out;
}

template <typename T>
nlohmann::json scalar_to_json(
    const rosidl_typesupport_introspection_cpp::MessageMember& m,
    const void* msg) {
    if (!m.is_array_) {
        return nlohmann::json(*static_cast<const T*>(field(m, msg, 0)));
    }

    nlohmann::json arr = nlohmann::json::array();
    const size_t n = element_count(m, msg);
    for (size_t i = 0; i < n; ++i) {
        arr.push_back(*static_cast<const T*>(field(m, msg, i)));
    }

    return arr;
}

template <typename T>
void scalar_from_json(
    const rosidl_typesupport_introspection_cpp::MessageMember& m, void* msg,
    const nlohmann::json& j) {
    if (!m.is_array_) {
        *static_cast<T*>(field(m, msg, 0)) = j.get<T>();
        return;
    }

    if (!j.is_array()) {
        unsupported("non-array value for array field " + std::string(m.name_));
    }

    if (!m.resize_function && j.size() != m.array_size_) {
        throw std::runtime_error(
            "msg_json: fixed array size mismatch for field " +
            std::string(m.name_));
    }

    if (m.resize_function) {
        m.resize_function(member_ptr(m, msg), j.size());
    }

    for (size_t i = 0; i < j.size(); ++i) {
        *static_cast<T*>(field(m, msg, i)) = j[i].get<T>();
    }
}

nlohmann::json string_to_json(
    const rosidl_typesupport_introspection_cpp::MessageMember& m,
    const void* msg) {
    if (!m.is_array_) {
        return nlohmann::json(
            *static_cast<const std::string*>(field(m, msg, 0)));
    }

    if (m.string_upper_bound_ != 0) {
        std::string s;
        const size_t n = element_count(m, msg);
        s.reserve(n);
        for (size_t i = 0; i < n; ++i) {
            s.push_back(*static_cast<const char*>(field(m, msg, i)));
        }
        return nlohmann::json(std::move(s));
    }

    nlohmann::json arr = nlohmann::json::array();
    const size_t n = element_count(m, msg);
    for (size_t i = 0; i < n; ++i) {
        arr.push_back(*static_cast<const std::string*>(field(m, msg, i)));
    }

    return arr;
}

void string_from_json(
    const rosidl_typesupport_introspection_cpp::MessageMember& m, void* msg,
    const nlohmann::json& j) {
    if (!m.is_array_) {
        *static_cast<std::string*>(field(m, msg, 0)) = j.get<std::string>();
        return;
    }

    if (m.string_upper_bound_ != 0) {
        const std::string s = j.get<std::string>();
        m.resize_function(member_ptr(m, msg), s.size());

        for (size_t i = 0; i < s.size(); ++i) {
            *static_cast<char*>(field(m, msg, i)) = s[i];
        }

        return;
    }

    if (!j.is_array()) {
        unsupported("non-array value for array field " + std::string(m.name_));
    }

    if (!m.resize_function && j.size() != m.array_size_) {
        throw std::runtime_error(
            "msg_json: fixed array size mismatch for field " +
            std::string(m.name_));
    }

    if (m.resize_function) {
        m.resize_function(member_ptr(m, msg), j.size());
    }

    for (size_t i = 0; i < j.size(); ++i) {
        *static_cast<std::string*>(field(m, msg, i)) = j[i].get<std::string>();
    }
}

nlohmann::json wstring_to_json(
    const rosidl_typesupport_introspection_cpp::MessageMember& m,
    const void* msg) {
    if (m.is_array_ || m.string_upper_bound_ != 0) {
        unsupported("wstring array");
    }

    return nlohmann::json(
        utf16_to_utf8(*static_cast<const std::u16string*>(field(m, msg, 0))));
}

void wstring_from_json(
    const rosidl_typesupport_introspection_cpp::MessageMember& m, void* msg,
    const nlohmann::json& j) {
    if (m.is_array_ || m.string_upper_bound_ != 0) {
        unsupported("wstring array");
    }

    *static_cast<std::u16string*>(field(m, msg, 0)) =
        utf8_to_utf16(j.get<std::string>());
}

nlohmann::json member_to_json(
    const rosidl_typesupport_introspection_cpp::MessageMember& m,
    const void* msg) {
    switch (m.type_id_) {
        case rosidl_typesupport_introspection_cpp::ROS_TYPE_BOOL:
            return scalar_to_json<bool>(m, msg);
        case rosidl_typesupport_introspection_cpp::ROS_TYPE_BYTE:
            return scalar_to_json<uint8_t>(m, msg);
        case rosidl_typesupport_introspection_cpp::ROS_TYPE_CHAR:
            return scalar_to_json<int8_t>(m, msg);
        case rosidl_typesupport_introspection_cpp::ROS_TYPE_WCHAR:
            return scalar_to_json<uint16_t>(m, msg);
        case rosidl_typesupport_introspection_cpp::ROS_TYPE_FLOAT32:
            return scalar_to_json<float>(m, msg);
        case rosidl_typesupport_introspection_cpp::ROS_TYPE_FLOAT64:
            return scalar_to_json<double>(m, msg);
        case rosidl_typesupport_introspection_cpp::ROS_TYPE_INT8:
            return scalar_to_json<int8_t>(m, msg);
        case rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT8:
            return scalar_to_json<uint8_t>(m, msg);
        case rosidl_typesupport_introspection_cpp::ROS_TYPE_INT16:
            return scalar_to_json<int16_t>(m, msg);
        case rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT16:
            return scalar_to_json<uint16_t>(m, msg);
        case rosidl_typesupport_introspection_cpp::ROS_TYPE_INT32:
            return scalar_to_json<int32_t>(m, msg);
        case rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT32:
            return scalar_to_json<uint32_t>(m, msg);
        case rosidl_typesupport_introspection_cpp::ROS_TYPE_INT64:
            return scalar_to_json<int64_t>(m, msg);
        case rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT64:
            return scalar_to_json<uint64_t>(m, msg);
        case rosidl_typesupport_introspection_cpp::ROS_TYPE_STRING:
            return string_to_json(m, msg);
        case rosidl_typesupport_introspection_cpp::ROS_TYPE_WSTRING:
            return wstring_to_json(m, msg);
        default:
            unsupported("field type");
    }
}

void member_from_json(
    const rosidl_typesupport_introspection_cpp::MessageMember& m, void* msg,
    const nlohmann::json& j) {
    switch (m.type_id_) {
        case rosidl_typesupport_introspection_cpp::ROS_TYPE_BOOL:
            return scalar_from_json<bool>(m, msg, j);
        case rosidl_typesupport_introspection_cpp::ROS_TYPE_BYTE:
            return scalar_from_json<uint8_t>(m, msg, j);
        case rosidl_typesupport_introspection_cpp::ROS_TYPE_CHAR:
            return scalar_from_json<int8_t>(m, msg, j);
        case rosidl_typesupport_introspection_cpp::ROS_TYPE_WCHAR:
            return scalar_from_json<uint16_t>(m, msg, j);
        case rosidl_typesupport_introspection_cpp::ROS_TYPE_FLOAT32:
            return scalar_from_json<float>(m, msg, j);
        case rosidl_typesupport_introspection_cpp::ROS_TYPE_FLOAT64:
            return scalar_from_json<double>(m, msg, j);
        case rosidl_typesupport_introspection_cpp::ROS_TYPE_INT8:
            return scalar_from_json<int8_t>(m, msg, j);
        case rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT8:
            return scalar_from_json<uint8_t>(m, msg, j);
        case rosidl_typesupport_introspection_cpp::ROS_TYPE_INT16:
            return scalar_from_json<int16_t>(m, msg, j);
        case rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT16:
            return scalar_from_json<uint16_t>(m, msg, j);
        case rosidl_typesupport_introspection_cpp::ROS_TYPE_INT32:
            return scalar_from_json<int32_t>(m, msg, j);
        case rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT32:
            return scalar_from_json<uint32_t>(m, msg, j);
        case rosidl_typesupport_introspection_cpp::ROS_TYPE_INT64:
            return scalar_from_json<int64_t>(m, msg, j);
        case rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT64:
            return scalar_from_json<uint64_t>(m, msg, j);
        case rosidl_typesupport_introspection_cpp::ROS_TYPE_STRING:
            return string_from_json(m, msg, j);
        case rosidl_typesupport_introspection_cpp::ROS_TYPE_WSTRING:
            return wstring_from_json(m, msg, j);
        default:
            unsupported("field type");
    }
}

struct to_frame {
    const rosidl_typesupport_introspection_cpp::MessageMembers* members;
    const void* msg;
    nlohmann::json* out;
};

struct from_frame {
    const rosidl_typesupport_introspection_cpp::MessageMembers* members;
    void* msg;
    const nlohmann::json* in;
};

}  // namespace

nlohmann::json to_json(
    const rosidl_typesupport_introspection_cpp::MessageMembers* members,
    const void* msg) {
    nlohmann::json root;
    std::stack<to_frame> stack;
    stack.push({members, msg, &root});

    while (!stack.empty()) {
        const to_frame frame = stack.top();
        stack.pop();

        nlohmann::json obj = nlohmann::json::object();
        for (uint32_t i = 0; i < frame.members->member_count_; ++i) {
            const rosidl_typesupport_introspection_cpp::MessageMember& m =
                frame.members->members_[i];

            if (m.type_id_ ==
                rosidl_typesupport_introspection_cpp::ROS_TYPE_MESSAGE) {
                if (!m.is_array_) {
                    obj[m.name_] = nlohmann::json::object();
                    stack.push({nested_members(m), field(m, frame.msg, 0),
                                     &obj[m.name_]});
                } else {
                    const size_t n = element_count(m, frame.msg);
                    nlohmann::json arr = nlohmann::json::array();
                    for (size_t k = 0; k < n; ++k) {
                        arr.push_back(nlohmann::json::object());
                    }

                    for (size_t k = 0; k < n; ++k) {
                        stack.push({nested_members(m),
                                         field(m, frame.msg, k), &arr[k]});
                    }

                    obj[m.name_] = std::move(arr);
                }

                continue;
            }

            obj[m.name_] = member_to_json(m, frame.msg);
        }

        *frame.out = std::move(obj);
    }

    return root;
}

void from_json(
    const rosidl_typesupport_introspection_cpp::MessageMembers* members,
    void* msg, const nlohmann::json& j) {
    std::stack<from_frame> stack;
    stack.push({members, msg, &j});

    while (!stack.empty()) {
        const from_frame frame = stack.top();
        stack.pop();

        if (!frame.in->is_object()) {
            throw std::runtime_error("msg_json: expected a JSON object");
        }

        for (uint32_t i = 0; i < frame.members->member_count_; ++i) {
            const rosidl_typesupport_introspection_cpp::MessageMember& m =
                frame.members->members_[i];
            auto it = frame.in->find(m.name_);
            if (it == frame.in->end()) {
                throw std::runtime_error("msg_json: missing field " +
                                         std::string(m.name_));
            }

            if (m.type_id_ ==
                rosidl_typesupport_introspection_cpp::ROS_TYPE_MESSAGE) {
                if (!m.is_array_) {
                    stack.push(
                        {nested_members(m), field(m, frame.msg, 0), &(*it)});
                } else {
                    if (!it->is_array()) {
                        unsupported("non-array value for array field " +
                                    std::string(m.name_));
                    }

                    if (!m.resize_function && it->size() != m.array_size_) {
                        throw std::runtime_error(
                            "msg_json: fixed array size mismatch for field " +
                            std::string(m.name_));
                    }

                    if (m.resize_function) {
                        m.resize_function(member_ptr(m, frame.msg), it->size());
                    }

                    for (size_t k = 0; k < it->size(); ++k) {
                        stack.push({nested_members(m),
                                         field(m, frame.msg, k), &(*it)[k]});
                    }
                }

                continue;
            }

            member_from_json(m, frame.msg, *it);
        }
    }
}

}  // namespace clover2_http_plugins::utils::msg_json::detail
