#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "fastwer.hpp"

namespace py = pybind11;

// See: https://realpython.com/python-bindings-overview/
PYBIND11_MODULE(fastwer, m) {
    m.def("score", &fastwer::score, "",py::arg("hypo"), py::arg("ref"), py::arg("char_level") = false);
    m.def("score_sent", &fastwer::score_sent, "",py::arg("hypo"), py::arg("ref"), py::arg("char_level") = false);
    m.def("bagOfWords", &fastwer::bagOfWords, "",py::arg("hypo"), py::arg("ref"), py::arg("char_level") = false);  // AHT
    m.def("compute", &fastwer::compute, "",py::arg("hypo"), py::arg("ref"), py::arg("char_level") = false);  // AHT
    m.def("alignment", &fastwer::alignment, "Return a list with the word/char alignment between hypothesis and reference strings. Each element of the list is a tuple of the form: (hyp_j, ref_i, cost).",py::arg("hypo"), py::arg("ref"), py::arg("char_level") = false);  // AHT
}
