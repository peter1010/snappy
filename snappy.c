#include <Python.h>
#include <structmember.h>

#define MAX_WORD_LEN   15     /* Max width of scramble board */

typedef struct {
    PyObject_HEAD
    char * data_buf;
    char ** word_lists[MAX_WORD_LEN+1];
} WordsObject;

/**
 * The Deallocator
 */
static void Words_dealloc(WordsObject * self)
{
    int i;
    for(i = 0; i <= MAX_WORD_LEN; i++) {
        free(self->word_lists[i]);
        self->word_lists[i] = 0;
    }
    free(self->data_buf);
    self->data_buf = 0;
}

/**
 * The Allocator
 */
static PyObject * Words_new(PyTypeObject * type, PyObject * args, PyObject * kwds)
{
    int i;
    WordsObject * self = (WordsObject *)type->tp_alloc(type, 0);
    if(self == NULL) {
        return NULL;
    }

    for(i = 0; i <= MAX_WORD_LEN; i++) {
        self->word_lists[i] = 0;
    }
    self->data_buf = 0;
    return self;
}


static char * skip_spaces(char * p)
{
    for(;(*p == ' ') || (*p == '\t'); p++)
        ;
    return p;
}



static int read_words(FILE * fp, int * word_cnts, char * data_buf)
{
    char lbuf[128];
    int num_words = 0;
    while(fgets(lbuf, sizeof(lbuf), fp)) {
        char * p = skip_spaces(&lbuf[0]);
        char * q;
        int word_len;
        for(q=p; *q; q++) {
            if((*q >= 'a') && (*q <= 'z')) {
                *q = *q - 'a' + 'A';
            }
            else if((*q < 'A') || (*q > 'Z')) {
                break;
            }
        }
        word_len = q-p;
        if(word_len <= MAX_WORD_LEN) {
            word_cnts[word_len]++;
            strncpy(data_buf, p, word_len);
            data_buf += word_len;
            *data_buf++ = '\0';
        }
    }
    *data_buf++ = '\0';
    return num_words;
}


/**
 * The Constructor
 */
static int Words_init(WordsObject * self, PyObject * args, PyObject * kwds)
{
    const char * filename;
    FILE * fp;
    long buf_len;
    int word_cnts[MAX_WORD_LEN+1];
    int num_words;

    if(!PyArg_ParseTuple(args, "s", &filename)) {
        /* Exception raised by PyArg_ParseTuple */
        return -1;
    }
    //filename="Lexicon.txt"):
    fp = fopen(filename, "r");
    if(fp == NULL) {
        PyErr_SetString(PyExc_FileExistsError, "Failed to open file");
        return -1;
    }
    fprintf(stderr, "Opened '%s'\n", filename);
    memset(word_cnts, 0, sizeof(word_cnts));

    fseek(fp, 0, SEEK_END);
    buf_len = sizeof(char) * ftell(fp);
    rewind(fp);

    self->data_buf = (char *)malloc(buf_len);

    printf("Creating dictionary, sorting words by length\n");
    num_words = read_words(fp, word_cnts, self->data_buf);
    fclose(fp);

    {
        int i;
        for(i = 0; i <= MAX_WORD_LEN; i++)
        {
            self->word_lists[i] = (char **)malloc(sizeof(char **) * (word_cnts[i]+1));
            printf("\t%i Words of length %i\n", word_cnts[i], i);
        }
    }
    return 0;
}


static PyMemberDef Words_members[] = {
    {NULL}  /* Sentinel */
};


static PyMethodDef Words_methods[] = {
    {NULL}  /* Sentinel */
};



static PyTypeObject WordsType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "words.Words",       /* tp_name */
    sizeof(WordsObject), /* tp_basicsize */
    0,                          /* tp_itemsize */
    (destructor) Words_dealloc, /* tp_dealloc */
    0,                          /* tp_print */
    0,                         /* tp_getattr */
    0,                         /* tp_setattr */
    0,                         /* tp_reserved */
    0,                         /* tp_repr */
    0,                         /* tp_as_number */
    0,                         /* tp_as_sequence */
    0,                         /* tp_as_mapping */
    0,                         /* tp_hash  */
    0,                         /* tp_call */
    0,                         /* tp_str */
    0,                         /* tp_getattro */
    0,                         /* tp_setattro */
    0,                         /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,        /* tp_flags */
    "Words objects",           /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    Words_methods,             /* tp_methods */
    Words_members,             /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)Words_init,      /* tp_init */
    0,                         /* tp_alloc */
    Words_new,                 /* tp_new */
};

static PyMethodDef WordsMethods[] = {
    {NULL, NULL, 0, NULL}    /* Sentinel */
};

static struct PyModuleDef wordsmodule = {
    PyModuleDef_HEAD_INIT,
    "words",    /* Name of module */
    NULL,       /* Module documentation */
    -1,         /* sizeof per-interpreter state */
    WordsMethods
};

PyMODINIT_FUNC PyInit_words(void)
{
    PyObject * mod;

    WordsType.tp_new = PyType_GenericNew;
    if(PyType_Ready(&WordsType) < 0)
    {
        return NULL;
    }
    mod = PyModule_Create(&wordsmodule);
    if(mod == NULL)
    {
        return NULL;
    }
    Py_INCREF(&WordsType);
    PyModule_AddObject(mod, "Words", (PyObject *)&WordsType);
    return mod;
}
