/* clang-format off */
/*[clinic input]
preserve
[clinic start generated code]*/

PyDoc_STRVAR(OrderedDict_fromkeys__doc__,
"fromkeys($type, /, iterable, value=None)\n"
"--\n"
"\n"
"New ordered dictionary with keys from S.\n"
"\n"
"If not specified, the value defaults to None.");

#define ORDEREDDICT_FROMKEYS_METHODDEF    \
    {"fromkeys", (PyCFunction)OrderedDict_fromkeys, METH_FASTCALL|METH_CLASS, OrderedDict_fromkeys__doc__},

static PyObject *
OrderedDict_fromkeys_impl(PyTypeObject *type, PyObject *seq, PyObject *value);

static PyObject *
OrderedDict_fromkeys(PyTypeObject *type, PyObject **args, Py_ssize_t nargs, PyObject *kwnames)
{
    PyObject *return_value = NULL;
    static const char * const _keywords[] = {"iterable", "value", NULL};
    static _PyArg_Parser _parser = {"O|O:fromkeys", _keywords, 0};
    PyObject *seq;
    PyObject *value = Py_None;

    if (!_PyArg_ParseStackAndKeywords(args, nargs, kwnames, &_parser,
        &seq, &value)) {
        goto exit;
    }
    return_value = OrderedDict_fromkeys_impl(type, seq, value);

exit:
    return return_value;
}

PyDoc_STRVAR(OrderedDict_setdefault__doc__,
"setdefault($self, /, key, default=None)\n"
"--\n"
"\n"
"od.get(k,d), also set od[k]=d if k not in od.");

#define ORDEREDDICT_SETDEFAULT_METHODDEF    \
    {"setdefault", (PyCFunction)OrderedDict_setdefault, METH_FASTCALL, OrderedDict_setdefault__doc__},

static PyObject *
OrderedDict_setdefault_impl(PyODictObject *self, PyObject *key,
                            PyObject *failobj);

static PyObject *
OrderedDict_setdefault(PyODictObject *self, PyObject **args, Py_ssize_t nargs, PyObject *kwnames)
{
    PyObject *return_value = NULL;
    static const char * const _keywords[] = {"key", "default", NULL};
    static _PyArg_Parser _parser = {"O|O:setdefault", _keywords, 0};
    PyObject *key;
    PyObject *failobj = Py_None;

    if (!_PyArg_ParseStackAndKeywords(args, nargs, kwnames, &_parser,
        &key, &failobj)) {
        goto exit;
    }
    return_value = OrderedDict_setdefault_impl(self, key, failobj);

exit:
    return return_value;
}

PyDoc_STRVAR(OrderedDict_popitem__doc__,
"popitem($self, /, last=True)\n"
"--\n"
"\n"
"Return (k, v) and remove a (key, value) pair.\n"
"\n"
"Pairs are returned in LIFO order if last is true or FIFO order if false.");

#define ORDEREDDICT_POPITEM_METHODDEF    \
    {"popitem", (PyCFunction)OrderedDict_popitem, METH_FASTCALL, OrderedDict_popitem__doc__},

static PyObject *
OrderedDict_popitem_impl(PyODictObject *self, int last);

static PyObject *
OrderedDict_popitem(PyODictObject *self, PyObject **args, Py_ssize_t nargs, PyObject *kwnames)
{
    PyObject *return_value = NULL;
    static const char * const _keywords[] = {"last", NULL};
    static _PyArg_Parser _parser = {"|p:popitem", _keywords, 0};
    int last = 1;

    if (!_PyArg_ParseStackAndKeywords(args, nargs, kwnames, &_parser,
        &last)) {
        goto exit;
    }
    return_value = OrderedDict_popitem_impl(self, last);

exit:
    return return_value;
}

PyDoc_STRVAR(OrderedDict_move_to_end__doc__,
"move_to_end($self, /, key, last=True)\n"
"--\n"
"\n"
"\"Move an existing element to the end (or beginning if last==False).\n"
"\n"
"    Raises KeyError if the element does not exist.\n"
"    When last=True, acts like a fast version of self[key]=self.pop(key).");

#define ORDEREDDICT_MOVE_TO_END_METHODDEF    \
    {"move_to_end", (PyCFunction)OrderedDict_move_to_end, METH_FASTCALL, OrderedDict_move_to_end__doc__},

static PyObject *
OrderedDict_move_to_end_impl(PyODictObject *self, PyObject *key, int last);

static PyObject *
OrderedDict_move_to_end(PyODictObject *self, PyObject **args, Py_ssize_t nargs, PyObject *kwnames)
{
    PyObject *return_value = NULL;
    static const char * const _keywords[] = {"key", "last", NULL};
    static _PyArg_Parser _parser = {"O|p:move_to_end", _keywords, 0};
    PyObject *key;
    int last = 1;

    if (!_PyArg_ParseStackAndKeywords(args, nargs, kwnames, &_parser,
        &key, &last)) {
        goto exit;
    }
    return_value = OrderedDict_move_to_end_impl(self, key, last);

exit:
    return return_value;
}
/*[clinic end generated code: output=f2641e1277045b59 input=a9049054013a1b77]*/
