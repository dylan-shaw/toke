#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <toke/decoder.h>
#include <toke/encoder.h>
#include <toke/model.h>

#include "exceptions.h"
#include "train.h"

#include <string>

#include <cstdint>
#include <cstdlib>
#include <cstring>

namespace py = pybind11;

namespace toke {

namespace {

void
throw_out_of_memory()
{
  throw std::runtime_error("out of memory");
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
    throw_if_error(err);
  }

  void parse_vocab(const std::string& vocab)
  {
    const auto err = toke_encoder_parse_vocab(m_self, vocab.data(), vocab.size());
    throw_if_error(err);
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
    throw_if_error(err);
  }

  void parse_vocab(const std::string& vocab)
  {
    const auto err = toke_decoder_parse_vocab(m_self, vocab.data(), vocab.size());
    throw_if_error(err);
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

class Model final
{
public:
  Model()
    : m_self(toke_model_new())
  {
    if (!m_self) {
      throw_out_of_memory();
    }
  }

  ~Model() { toke_model_delete(m_self); }

  void add(const std::string& data)
  {
    const auto err = toke_model_add_token(m_self, reinterpret_cast<const uint8_t*>(data.data()), data.size());
    throw_if_error(err);
  }

  void add_unicode_block(const toke_unicode_block_z block)
  {
    const auto err = toke_model_add_unicode_block(m_self, block);
    throw_if_error(err);
  }

  [[nodiscard]] auto at(std::size_t index) const -> std::string
  {
    const uint8_t* data = toke_model_get_def(m_self, index);
    const std::size_t size = toke_model_get_size(m_self, index);
    return std::string(reinterpret_cast<const char*>(data), size);
  }

  [[nodiscard]] auto size() const -> size_t { return toke_model_size(m_self); }

private:
  toke_model_z* m_self{};
};

} // namespace

} // namespace toke

PYBIND11_MODULE(toke, m)
{
  py::class_<toke::Encoder>(m, "Encoder")
    .def(py::init<>())
    .def("load_vocab", &toke::Encoder::load_vocab, py::arg("filename"))
    .def("parse_vocab", &toke::Encoder::parse_vocab, py::arg("vocab"))
    .def("encode", &toke::Encoder::encode, py::arg("text"));

  py::class_<toke::Decoder>(m, "Decoder")
    .def(py::init<>())
    .def("load_vocab", &toke::Decoder::load_vocab, py::arg("filename"))
    .def("parse_vocab", &toke::Decoder::parse_vocab, py::arg("vocab"))
    .def("decode", &toke::Decoder::decode, py::arg("tokens"));

  py::enum_<toke_unicode_block_z>(m, "UnicodeBlock")
    .value("BASIC_LATIN", TOKE_UNICODE_BLOCK_BASIC_LATIN)
    .value("GENERAL_PUNCTUATION", TOKE_UNICODE_BLOCK_GENERAL_PUNCTUATION);

  py::class_<toke::Model>(m, "Model")
    .def(py::init<>())
    .def("__len__", &toke::Model::size)
    .def("__getitem__", &toke::Model::at, py::arg("index"))
    .def("add", &toke::Model::add, py::arg("data"))
    .def("add_unicode_block", &toke::Model::add_unicode_block, py::arg("block"));
  ;

  toke::def_train_model(m);
}
