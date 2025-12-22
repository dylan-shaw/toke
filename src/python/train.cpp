#include "train.h"

#include <toke/train/dataset.h>

#include "exceptions.h"

namespace toke {

namespace {

namespace py = pybind11;

class Dataset final
{
public:
  Dataset()
    : m_self(toke_dataset_new())
  {
    if (!m_self) {
      throw std::runtime_error("failed to allocate dataset");
    }
  }

  ~Dataset() { toke_dataset_delete(m_self); }

  void open(const char* filename)
  {
    const auto err = toke_dataset_open(m_self, filename);
    throw_if_error(err);
    toke_dataset_walk(m_self, 1, nullptr, nullptr);
  }

private:
  toke_dataset_z* m_self{};
};

} // namespace

void
def_train_model(pybind11::module_& parent_m)
{
  auto m = parent_m.def_submodule("train", "Used for training new tokenizers.");

  py::class_<Dataset>(m, "Dataset").def(py::init<>()).def("open", &Dataset::open, py::arg("filename"));
}

} // namespace toke
