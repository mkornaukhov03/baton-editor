#ifndef BATON_JSON_SERIALIZERS_H
#define BATON_JSON_SERIALIZERS_H

namespace nlohmann {
template <typename T>
struct adl_serializer<lsp::optional<T>> {
static void to_json(json &j, const lsp::optional<T> &opt) {
  if (opt) {
    j = opt.value();
  } else {
    j = nullptr;
  }
}
static void from_json(const json &j, lsp::optional<T> &opt) {
  if (j.is_null()) {
    opt = lsp::optional<T>();
  } else {
    opt = lsp::optional<T>(j.get<T>());
  }
}
};
template <>
struct adl_serializer<lsp::URIForFile> {
  static void to_json(json &j, const lsp::URIForFile &uri) { j = uri.str(); }
  static void from_json(const json &j, lsp::URIForFile &uri) {
    uri.set_from_encoded(j.get<std::string>());
  }
};
}  // namespace nlohmann

#endif  // BATON_JSON_SERIALIZERS_H
