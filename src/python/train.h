#pragma once

#include <pybind11/pybind11.h>

namespace toke {

void
def_train_model(pybind11::module_& m);

} // namespace toke
