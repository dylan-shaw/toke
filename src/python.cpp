#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <toke/decoder.h>
#include <toke/encoder.h>

#include <cstdint>
#include <cstring>

namespace {

namespace py = pybind11;

void
throw_out_of_memory()
{
  throw std::runtime_error("out of memory");
}

void
check_error(const toke_error_z err)
{
  if (err == TOKE_ERROR_NONE) {
    return;
  }

  throw std::runtime_error(toke_strerror(err));
}

class Encoder final
{
public:
  Encoder()
    : m_self(toke_encoder_new())
  {
    if (!m_self) {
      throw_out_of_memory();
    }
  }

  ~Encoder() { toke_encoder_delete(m_self); }

  void load_vocab(const std::string& filename)
  {
    const auto err = toke_encoder_load_vocab(m_self, filename.c_str());
    check_error(err);
  }

  void parse_vocab(const std::string& vocab)
  {
    const auto err = toke_encoder_parse_vocab(m_self, vocab.data(), vocab.size());
    check_error(err);
  }

  [[nodiscard]] auto encode(const std::string& txt) const
    -> py::array_t<std::uint16_t, py::array::forcecast | py::array::c_style>
  {
    size_t out_size = 0;

    auto* out_ptr = toke_encode(m_self, txt.data(), txt.size(), &out_size);
    if (!out_ptr) {
      throw_out_of_memory();
    }

    py::array_t<std::uint16_t, py::array::forcecast | py::array::c_style> result;

    result.resize(std::array<py::ssize_t, 1>{ static_cast<py::ssize_t>(out_size) });

    std::memcpy(result.mutable_data(0), out_ptr, out_size * sizeof(std::uint16_t));

    std::free(out_ptr);

    return result;
  }

private:
  toke_encoder_z* m_self{};
};

class Decoder final
{
public:
  Decoder()
    : m_self(toke_decoder_new())
  {
    if (!m_self) {
      throw_out_of_memory();
    }
  }

  ~Decoder() { toke_decoder_delete(m_self); }

  void load_vocab(const std::string& filename)
  {
    const auto err = toke_decoder_load_vocab(m_self, filename.c_str());
    check_error(err);
  }

  void parse_vocab(const std::string& vocab)
  {
    const auto err = toke_decoder_parse_vocab(m_self, vocab.data(), vocab.size());
    check_error(err);
  }

  [[nodiscard]] auto decode(const py::array_t<std::uint16_t, py::array::forcecast | py::array::c_style>& tokens) const
    -> std::string
  {
    size_t out_size = 0;

    char* data = toke_decode(m_self, tokens.data(0), tokens.size(), &out_size);
    if (!data) {
      throw_out_of_memory();
    }

    std::string tmp(data, out_size);

    std::free(data);

    return tmp;
  }

private:
  toke_decoder_z* m_self{};
};

} // namespace

PYBIND11_MODULE(pytoke, m)
{
  py::class_<Encoder>(m, "Encoder")
    .def(py::init<>())
    .def("load_vocab", &Encoder::load_vocab, py::arg("filename"))
    .def("parse_vocab", &Encoder::parse_vocab, py::arg("vocab"))
    .def("encode", &Encoder::encode, py::arg("text"));

  py::class_<Decoder>(m, "Decoder")
    .def(py::init<>())
    .def("load_vocab", &Decoder::load_vocab, py::arg("filename"))
    .def("parse_vocab", &Decoder::parse_vocab, py::arg("vocab"))
    .def("decode", &Decoder::decode, py::arg("tokens"));
}
