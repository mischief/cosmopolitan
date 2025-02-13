/* clang-format off */
/*[clinic input]
preserve
[clinic start generated code]*/

PyDoc_STRVAR(Struct___init____doc__,
"Struct(format)\n"
"--\n"
"\n"
"Create a compiled struct object.\n"
"\n"
"Return a new Struct object which writes and reads binary data according to\n"
"the format string.\n"
"\n"
"See help(struct) for more on format strings.");

static int
Struct___init___impl(PyStructObject *self, PyObject *format);

static int
Struct___init__(PyObject *self, PyObject *args, PyObject *kwargs)
{
    int return_value = -1;
    static const char * const _keywords[] = {"format", NULL};
    static _PyArg_Parser _parser = {"O:Struct", _keywords, 0};
    PyObject *format;

    if (!_PyArg_ParseTupleAndKeywordsFast(args, kwargs, &_parser,
        &format)) {
        goto exit;
    }
    return_value = Struct___init___impl((PyStructObject *)self, format);

exit:
    return return_value;
}

PyDoc_STRVAR(Struct_unpack__doc__,
"unpack($self, buffer, /)\n"
"--\n"
"\n"
"Return a tuple containing unpacked values.\n"
"\n"
"Unpack according to the format string Struct.format. The buffer\'s size\n"
"in bytes must be Struct.size.\n"
"\n"
"See help(struct) for more on format strings.");

#define STRUCT_UNPACK_METHODDEF    \
    {"unpack", (PyCFunction)Struct_unpack, METH_O, Struct_unpack__doc__},

static PyObject *
Struct_unpack_impl(PyStructObject *self, Py_buffer *buffer);

static PyObject *
Struct_unpack(PyStructObject *self, PyObject *arg)
{
    PyObject *return_value = NULL;
    Py_buffer buffer = {NULL, NULL};

    if (!PyArg_Parse(arg, "y*:unpack", &buffer)) {
        goto exit;
    }
    return_value = Struct_unpack_impl(self, &buffer);

exit:
    /* Cleanup for buffer */
    if (buffer.obj) {
       PyBuffer_Release(&buffer);
    }

    return return_value;
}

PyDoc_STRVAR(Struct_unpack_from__doc__,
"unpack_from($self, /, buffer, offset=0)\n"
"--\n"
"\n"
"Return a tuple containing unpacked values.\n"
"\n"
"Values are unpacked according to the format string Struct.format.\n"
"\n"
"The buffer\'s size in bytes, minus offset, must be at least Struct.size.\n"
"\n"
"See help(struct) for more on format strings.");

#define STRUCT_UNPACK_FROM_METHODDEF    \
    {"unpack_from", (PyCFunction)Struct_unpack_from, METH_FASTCALL, Struct_unpack_from__doc__},

static PyObject *
Struct_unpack_from_impl(PyStructObject *self, Py_buffer *buffer,
                        Py_ssize_t offset);

static PyObject *
Struct_unpack_from(PyStructObject *self, PyObject **args, Py_ssize_t nargs, PyObject *kwnames)
{
    PyObject *return_value = NULL;
    static const char * const _keywords[] = {"buffer", "offset", NULL};
    static _PyArg_Parser _parser = {"y*|n:unpack_from", _keywords, 0};
    Py_buffer buffer = {NULL, NULL};
    Py_ssize_t offset = 0;

    if (!_PyArg_ParseStackAndKeywords(args, nargs, kwnames, &_parser,
        &buffer, &offset)) {
        goto exit;
    }
    return_value = Struct_unpack_from_impl(self, &buffer, offset);

exit:
    /* Cleanup for buffer */
    if (buffer.obj) {
       PyBuffer_Release(&buffer);
    }

    return return_value;
}

PyDoc_STRVAR(Struct_iter_unpack__doc__,
"iter_unpack($self, buffer, /)\n"
"--\n"
"\n"
"Return an iterator yielding tuples.\n"
"\n"
"Tuples are unpacked from the given bytes source, like a repeated\n"
"invocation of unpack_from().\n"
"\n"
"Requires that the bytes length be a multiple of the struct size.");

#define STRUCT_ITER_UNPACK_METHODDEF    \
    {"iter_unpack", (PyCFunction)Struct_iter_unpack, METH_O, Struct_iter_unpack__doc__},

PyDoc_STRVAR(_clearcache__doc__,
"_clearcache($module, /)\n"
"--\n"
"\n"
"Clear the internal cache.");

#define _CLEARCACHE_METHODDEF    \
    {"_clearcache", (PyCFunction)_clearcache, METH_NOARGS, _clearcache__doc__},

static PyObject *
_clearcache_impl(PyObject *module);

static PyObject *
_clearcache(PyObject *module, PyObject *Py_UNUSED(ignored))
{
    return _clearcache_impl(module);
}

PyDoc_STRVAR(calcsize__doc__,
"calcsize($module, format, /)\n"
"--\n"
"\n"
"Return size in bytes of the struct described by the format string.");

#define CALCSIZE_METHODDEF    \
    {"calcsize", (PyCFunction)calcsize, METH_O, calcsize__doc__},

PyDoc_STRVAR(unpack__doc__,
"unpack($module, format, inputstr, /)\n"
"--\n"
"\n"
"Return a tuple containing values unpacked according to the format string.\n"
"\n"
"The buffer\'s size in bytes must be calcsize(format).\n"
"\n"
"See help(struct) for more on format strings.");

#define UNPACK_METHODDEF    \
    {"unpack", (PyCFunction)unpack, METH_FASTCALL, unpack__doc__},

static PyObject *
unpack_impl(PyObject *module, PyObject *format, PyObject *inputstr);

static PyObject *
unpack(PyObject *module, PyObject **args, Py_ssize_t nargs, PyObject *kwnames)
{
    PyObject *return_value = NULL;
    PyObject *format;
    PyObject *inputstr;

    if (!_PyArg_UnpackStack(args, nargs, "unpack",
        2, 2,
        &format, &inputstr)) {
        goto exit;
    }

    if (!_PyArg_NoStackKeywords("unpack", kwnames)) {
        goto exit;
    }
    return_value = unpack_impl(module, format, inputstr);

exit:
    return return_value;
}

PyDoc_STRVAR(unpack_from__doc__,
"unpack_from($module, format, /, buffer, offset=0)\n"
"--\n"
"\n"
"Return a tuple containing values unpacked according to the format string.\n"
"\n"
"The buffer\'s size, minus offset, must be at least calcsize(format).\n"
"\n"
"See help(struct) for more on format strings.");

#define UNPACK_FROM_METHODDEF    \
    {"unpack_from", (PyCFunction)unpack_from, METH_FASTCALL, unpack_from__doc__},

static PyObject *
unpack_from_impl(PyObject *module, PyObject *format, Py_buffer *buffer,
                 Py_ssize_t offset);

static PyObject *
unpack_from(PyObject *module, PyObject **args, Py_ssize_t nargs, PyObject *kwnames)
{
    PyObject *return_value = NULL;
    static const char * const _keywords[] = {"", "buffer", "offset", NULL};
    static _PyArg_Parser _parser = {"Oy*|n:unpack_from", _keywords, 0};
    PyObject *format;
    Py_buffer buffer = {NULL, NULL};
    Py_ssize_t offset = 0;

    if (!_PyArg_ParseStackAndKeywords(args, nargs, kwnames, &_parser,
        &format, &buffer, &offset)) {
        goto exit;
    }
    return_value = unpack_from_impl(module, format, &buffer, offset);

exit:
    /* Cleanup for buffer */
    if (buffer.obj) {
       PyBuffer_Release(&buffer);
    }

    return return_value;
}

PyDoc_STRVAR(iter_unpack__doc__,
"iter_unpack($module, format, buffer, /)\n"
"--\n"
"\n"
"Return an iterator yielding tuples unpacked from the given bytes.\n"
"\n"
"The bytes are unpacked according to the format string, like\n"
"a repeated invocation of unpack_from().\n"
"\n"
"Requires that the bytes length be a multiple of the format struct size.");

#define ITER_UNPACK_METHODDEF    \
    {"iter_unpack", (PyCFunction)iter_unpack, METH_FASTCALL, iter_unpack__doc__},

static PyObject *
iter_unpack_impl(PyObject *module, PyObject *format, PyObject *buffer);

static PyObject *
iter_unpack(PyObject *module, PyObject **args, Py_ssize_t nargs, PyObject *kwnames)
{
    PyObject *return_value = NULL;
    PyObject *format;
    PyObject *buffer;

    if (!_PyArg_UnpackStack(args, nargs, "iter_unpack",
        2, 2,
        &format, &buffer)) {
        goto exit;
    }

    if (!_PyArg_NoStackKeywords("iter_unpack", kwnames)) {
        goto exit;
    }
    return_value = iter_unpack_impl(module, format, buffer);

exit:
    return return_value;
}
/*[clinic end generated code: output=db8152ad222fa3d0 input=a9049054013a1b77]*/
