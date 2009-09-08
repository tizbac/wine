/*
 * Copyright 2009 Henri Verbeet for CodeWeavers
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 *
 */

#include "config.h"
#include "wine/port.h"

#include "d3d10_private.h"

WINE_DEFAULT_DEBUG_CHANNEL(d3d10);

#define MAKE_TAG(ch0, ch1, ch2, ch3) \
    ((DWORD)(ch0) | ((DWORD)(ch1) << 8) | \
    ((DWORD)(ch2) << 16) | ((DWORD)(ch3) << 24 ))
#define TAG_DXBC MAKE_TAG('D', 'X', 'B', 'C')
#define TAG_FX10 MAKE_TAG('F', 'X', '1', '0')
#define TAG_ISGN MAKE_TAG('I', 'S', 'G', 'N')

#define D3D10_FX10_TYPE_COLUMN_SHIFT    11
#define D3D10_FX10_TYPE_COLUMN_MASK     (0x7 << D3D10_FX10_TYPE_COLUMN_SHIFT)

#define D3D10_FX10_TYPE_ROW_SHIFT       8
#define D3D10_FX10_TYPE_ROW_MASK        (0x7 << D3D10_FX10_TYPE_ROW_SHIFT)

#define D3D10_FX10_TYPE_BASETYPE_SHIFT  3
#define D3D10_FX10_TYPE_BASETYPE_MASK   (0x1f << D3D10_FX10_TYPE_BASETYPE_SHIFT)

#define D3D10_FX10_TYPE_CLASS_SHIFT     0
#define D3D10_FX10_TYPE_CLASS_MASK      (0x7 << D3D10_FX10_TYPE_CLASS_SHIFT)

static const struct ID3D10EffectTechniqueVtbl d3d10_effect_technique_vtbl;
static const struct ID3D10EffectPassVtbl d3d10_effect_pass_vtbl;
static const struct ID3D10EffectVariableVtbl d3d10_effect_variable_vtbl;
static const struct ID3D10EffectConstantBufferVtbl d3d10_effect_constant_buffer_vtbl;
static const struct ID3D10EffectTypeVtbl d3d10_effect_type_vtbl;

/* null objects - needed for invalid calls */
static struct d3d10_effect_technique null_technique = {&d3d10_effect_technique_vtbl, NULL, NULL, 0, 0, NULL};
static struct d3d10_effect_pass null_pass = {&d3d10_effect_pass_vtbl, NULL, NULL, 0, 0, 0, NULL};
static struct d3d10_effect_local_buffer null_local_buffer =
        {&d3d10_effect_constant_buffer_vtbl, NULL, NULL, 0, 0, 0, NULL};
static struct d3d10_effect_variable null_variable = {&d3d10_effect_variable_vtbl, NULL, NULL, 0, 0, 0, NULL};

static inline void read_dword(const char **ptr, DWORD *d)
{
    memcpy(d, *ptr, sizeof(*d));
    *ptr += sizeof(*d);
}

static inline void skip_dword_unknown(const char **ptr, unsigned int count)
{
    unsigned int i;
    DWORD d;

    FIXME("Skipping %u unknown DWORDs:\n", count);
    for (i = 0; i < count; ++i)
    {
        read_dword(ptr, &d);
        FIXME("\t0x%08x\n", d);
    }
}

static inline void write_dword(char **ptr, DWORD d)
{
    memcpy(*ptr, &d, sizeof(d));
    *ptr += sizeof(d);
}

static inline void write_dword_unknown(char **ptr, DWORD d)
{
    FIXME("Writing unknown DWORD 0x%08x\n", d);
    write_dword(ptr, d);
}

static inline void read_tag(const char **ptr, DWORD *t, char t_str[5])
{
    read_dword(ptr, t);
    memcpy(t_str, t, 4);
    t_str[4] = '\0';
}

static HRESULT parse_dxbc(const char *data, SIZE_T data_size,
        HRESULT (*chunk_handler)(const char *data, DWORD data_size, DWORD tag, void *ctx), void *ctx)
{
    const char *ptr = data;
    HRESULT hr = S_OK;
    DWORD chunk_count;
    DWORD total_size;
    char tag_str[5];
    unsigned int i;
    DWORD tag;

    read_tag(&ptr, &tag, tag_str);
    TRACE("tag: %s\n", tag_str);

    if (tag != TAG_DXBC)
    {
        WARN("Wrong tag.\n");
        return E_FAIL;
    }

    /* checksum? */
    skip_dword_unknown(&ptr, 4);

    skip_dword_unknown(&ptr, 1);

    read_dword(&ptr, &total_size);
    TRACE("total size: %#x\n", total_size);

    read_dword(&ptr, &chunk_count);
    TRACE("chunk count: %#x\n", chunk_count);

    for (i = 0; i < chunk_count; ++i)
    {
        DWORD chunk_tag, chunk_size;
        const char *chunk_ptr;
        DWORD chunk_offset;

        read_dword(&ptr, &chunk_offset);
        TRACE("chunk %u at offset %#x\n", i, chunk_offset);

        chunk_ptr = data + chunk_offset;

        read_dword(&chunk_ptr, &chunk_tag);
        read_dword(&chunk_ptr, &chunk_size);

        hr = chunk_handler(chunk_ptr, chunk_size, chunk_tag, ctx);
        if (FAILED(hr)) break;
    }

    return hr;
}

static char *copy_name(const char *ptr)
{
    size_t name_len;
    char *name;

    name_len = strlen(ptr) + 1;
    name = HeapAlloc(GetProcessHeap(), 0, name_len);
    if (!name)
    {
        ERR("Failed to allocate name memory.\n");
        return NULL;
    }

    memcpy(name, ptr, name_len);

    return name;
}

static HRESULT shader_chunk_handler(const char *data, DWORD data_size, DWORD tag, void *ctx)
{
    struct d3d10_effect_shader_variable *s = ctx;
    char tag_str[5];

    memcpy(tag_str, &tag, 4);
    tag_str[4] = '\0';
    TRACE("tag: %s\n", tag_str);

    TRACE("chunk size: %#x\n", data_size);

    switch(tag)
    {
        case TAG_ISGN:
        {
            /* 32 (DXBC header) + 1 * 4 (chunk index) + 2 * 4 (chunk header) + data_size (chunk data) */
            UINT size = 44 + data_size;
            char *ptr;

            s->input_signature = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size);
            if (!s->input_signature)
            {
                ERR("Failed to allocate input signature data\n");
                return E_OUTOFMEMORY;
            }
            s->input_signature_size = size;

            ptr = s->input_signature;

            write_dword(&ptr, TAG_DXBC);

            /* signature(?) */
            write_dword_unknown(&ptr, 0);
            write_dword_unknown(&ptr, 0);
            write_dword_unknown(&ptr, 0);
            write_dword_unknown(&ptr, 0);

            /* seems to be always 1 */
            write_dword_unknown(&ptr, 1);

            /* DXBC size */
            write_dword(&ptr, size);

            /* chunk count */
            write_dword(&ptr, 1);

            /* chunk index */
            write_dword(&ptr, (ptr - s->input_signature) + 4);

            /* chunk */
            write_dword(&ptr, TAG_ISGN);
            write_dword(&ptr, data_size);
            memcpy(ptr, data, data_size);
            break;
        }

        default:
            FIXME("Unhandled chunk %s\n", tag_str);
            break;
    }

    return S_OK;
}

static HRESULT parse_shader(struct d3d10_effect_object *o, const char *data)
{
    ID3D10Device *device = o->pass->technique->effect->device;
    struct d3d10_effect_shader_variable *s;
    const char *ptr = data;
    DWORD dxbc_size;
    HRESULT hr;

    o->data = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(struct d3d10_effect_shader_variable));
    if (!o->data)
    {
        ERR("Failed to allocate shader variable memory\n");
        return E_OUTOFMEMORY;
    }

    if (!ptr) return S_OK;

    s = o->data;

    read_dword(&ptr, &dxbc_size);
    TRACE("dxbc size: %#x\n", dxbc_size);

    switch (o->type)
    {
        case D3D10_EOT_VERTEXSHADER:
            hr = ID3D10Device_CreateVertexShader(device, ptr, dxbc_size, &s->shader.vs);
            if (FAILED(hr)) return hr;
            break;

        case D3D10_EOT_PIXELSHADER:
            hr = ID3D10Device_CreatePixelShader(device, ptr, dxbc_size, &s->shader.ps);
            if (FAILED(hr)) return hr;
            break;
        case D3D10_EOT_GEOMETRYSHADER:
            hr = ID3D10Device_CreateGeometryShader(device, ptr, dxbc_size, &s->shader.gs);
            if (FAILED(hr)) return hr;
            break;
    }

    return parse_dxbc(ptr, dxbc_size, shader_chunk_handler, s);
}

static void parse_fx10_annotation(const char **ptr)
{
    skip_dword_unknown(ptr, 3);
}

static HRESULT parse_fx10_object(struct d3d10_effect_object *o, const char **ptr, const char *data)
{
    const char *data_ptr;
    DWORD offset;
    HRESULT hr;

    read_dword(ptr, &o->type);
    TRACE("Effect object is of type %#x.\n", o->type);

    skip_dword_unknown(ptr, 2);

    read_dword(ptr, &offset);
    TRACE("Effect object idx is at offset %#x.\n", offset);

    data_ptr = data + offset;
    read_dword(&data_ptr, &offset);

    TRACE("Effect object starts at offset %#x.\n", offset);

    /* FIXME: This probably isn't completely correct. */
    if (offset == 1)
    {
        WARN("Skipping effect object.\n");
        data_ptr = NULL;
    }
    else
    {
        data_ptr = data + offset;
    }

    switch (o->type)
    {
        case D3D10_EOT_VERTEXSHADER:
            TRACE("Vertex shader\n");
            hr = parse_shader(o, data_ptr);
            break;

        case D3D10_EOT_PIXELSHADER:
            TRACE("Pixel shader\n");
            hr = parse_shader(o, data_ptr);
            break;

        case D3D10_EOT_GEOMETRYSHADER:
            TRACE("Geometry shader\n");
            hr = parse_shader(o, data_ptr);
            break;

        default:
            FIXME("Unhandled object type %#x\n", o->type);
            hr = E_FAIL;
            break;
    }

    return hr;
}

static HRESULT parse_fx10_pass(struct d3d10_effect_pass *p, const char **ptr, const char *data)
{
    HRESULT hr = S_OK;
    unsigned int i;
    DWORD offset;

    read_dword(ptr, &offset);
    TRACE("Pass name at offset %#x.\n", offset);

    p->name = copy_name(data + offset);
    if (!p->name)
    {
        ERR("Failed to copy name.\n");
        return E_OUTOFMEMORY;
    }
    TRACE("Pass name: %s.\n", p->name);

    read_dword(ptr, &p->object_count);
    TRACE("Pass has %u effect objects.\n", p->object_count);

    read_dword(ptr, &p->annotation_count);
    for(i = 0; i < p->annotation_count; ++i)
    {
        parse_fx10_annotation(ptr);
    }

    p->objects = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, p->object_count * sizeof(*p->objects));
    if (!p->objects)
    {
        ERR("Failed to allocate effect objects memory.\n");
        return E_OUTOFMEMORY;
    }

    for (i = 0; i < p->object_count; ++i)
    {
        struct d3d10_effect_object *o = &p->objects[i];

        o->pass = p;

        hr = parse_fx10_object(o, ptr, data);
        if (FAILED(hr)) return hr;
    }

    return hr;
}

static HRESULT parse_fx10_technique(struct d3d10_effect_technique *t, const char **ptr, const char *data)
{
    unsigned int i;
    DWORD offset;

    read_dword(ptr, &offset);
    TRACE("Technique name at offset %#x.\n", offset);

    t->name = copy_name(data + offset);
    if (!t->name)
    {
        ERR("Failed to copy name.\n");
        return E_OUTOFMEMORY;
    }
    TRACE("Technique name: %s.\n", t->name);

    read_dword(ptr, &t->pass_count);
    TRACE("Technique has %u passes\n", t->pass_count);

    read_dword(ptr, &t->annotation_count);
    for(i = 0; i < t->annotation_count; ++i)
    {
        parse_fx10_annotation(ptr);
    }

    t->passes = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, t->pass_count * sizeof(*t->passes));
    if (!t->passes)
    {
        ERR("Failed to allocate passes memory\n");
        return E_OUTOFMEMORY;
    }

    for (i = 0; i < t->pass_count; ++i)
    {
        struct d3d10_effect_pass *p = &t->passes[i];
        HRESULT hr;

        p->vtbl = &d3d10_effect_pass_vtbl;
        p->technique = t;

        hr = parse_fx10_pass(p, ptr, data);
        if (FAILED(hr)) return hr;
    }

    return S_OK;
}

static D3D10_SHADER_VARIABLE_CLASS d3d10_variable_class(DWORD c)
{
    switch (c)
    {
        case 1: return D3D10_SVC_SCALAR;
        case 2: return D3D10_SVC_VECTOR;
        case 3: return D3D10_SVC_MATRIX_ROWS;
        default:
            FIXME("Unknown variable class %#x.\n", c);
            return 0;
    }
}

static D3D10_SHADER_VARIABLE_TYPE d3d10_variable_type(DWORD t)
{
    switch (t)
    {
        case 1: return D3D10_SVT_FLOAT;
        case 2: return D3D10_SVT_INT;
        case 3: return D3D10_SVT_UINT;
        case 4: return D3D10_SVT_BOOL;
        default:
            FIXME("Unknown variable type %#x.\n", t);
            return 0;
    }
}

static HRESULT parse_fx10_type(struct d3d10_effect_type *t, const char *ptr, const char *data)
{
    DWORD unknown0;
    DWORD offset;

    read_dword(&ptr, &offset);
    TRACE("Type name at offset %#x.\n", offset);

    t->name = copy_name(data + offset);
    if (!t->name)
    {
        ERR("Failed to copy name.\n");
        return E_OUTOFMEMORY;
    }
    TRACE("Type name: %s.\n", debugstr_a(t->name));

    read_dword(&ptr, &unknown0);
    TRACE("Unknown 0: %u.\n", unknown0);

    read_dword(&ptr, &t->element_count);
    TRACE("Element count: %u.\n", t->element_count);

    read_dword(&ptr, &t->size_unpacked);
    TRACE("Unpacked size: %#x.\n", t->size_unpacked);

    read_dword(&ptr, &t->stride);
    TRACE("Stride: %#x.\n", t->stride);

    read_dword(&ptr, &t->size_packed);
    TRACE("Packed size %#x.\n", t->size_packed);

    if (unknown0 == 1)
    {
        DWORD tmp;

        t->member_count = 0;

        read_dword(&ptr, &tmp);
        t->column_count = (tmp & D3D10_FX10_TYPE_COLUMN_MASK) >> D3D10_FX10_TYPE_COLUMN_SHIFT;
        t->row_count = (tmp & D3D10_FX10_TYPE_ROW_MASK) >> D3D10_FX10_TYPE_ROW_SHIFT;
        t->basetype = d3d10_variable_type((tmp & D3D10_FX10_TYPE_BASETYPE_MASK) >> D3D10_FX10_TYPE_BASETYPE_SHIFT);
        t->type_class = d3d10_variable_class((tmp & D3D10_FX10_TYPE_CLASS_MASK) >> D3D10_FX10_TYPE_CLASS_SHIFT);

        TRACE("Type description: %#x.\n", tmp);
        TRACE("\tcolumns: %u.\n", t->column_count);
        TRACE("\trows: %u.\n", t->row_count);
        TRACE("\tbasetype: %#x.\n", t->basetype);
        TRACE("\tclass: %#x.\n", t->type_class);
        TRACE("\tunknown bits: %#x.\n", tmp & ~(D3D10_FX10_TYPE_COLUMN_MASK | D3D10_FX10_TYPE_ROW_MASK
                | D3D10_FX10_TYPE_BASETYPE_MASK | D3D10_FX10_TYPE_CLASS_MASK));
    }
    else if (unknown0 == 3)
    {
        unsigned int i;
        DWORD tmp;

        TRACE("Type is a structure.\n");

        read_dword(&ptr, &t->member_count);
        TRACE("Member count: %u.\n", t->member_count);

        t->column_count = 0;
        t->row_count = 0;
        t->basetype = 0;
        t->type_class = D3D10_SVC_STRUCT;

        for (i = 0; i < t->member_count; ++i)
        {
            read_dword(&ptr, &tmp);
            TRACE("Member %u name at offset %#x.\n", i, tmp);
            TRACE("Member %u name: %s.\n", i, debugstr_a(data + tmp));

            /* Member semantic? */
            skip_dword_unknown(&ptr, 1);

            read_dword(&ptr, &tmp);
            TRACE("Member %u offset in struct: %#x.\n", i, tmp);

            read_dword(&ptr, &tmp);
            TRACE("Member %u type info at offset %#x.\n", i, tmp);
        }
    }

    return S_OK;
}

static struct d3d10_effect_type *get_fx10_type(struct d3d10_effect *effect, const char *data, DWORD offset)
{
    struct d3d10_effect_type *type;
    struct wine_rb_entry *entry;
    HRESULT hr;

    entry = wine_rb_get(&effect->types, &offset);
    if (entry)
    {
        TRACE("Returning existing type.\n");
        return WINE_RB_ENTRY_VALUE(entry, struct d3d10_effect_type, entry);
    }

    type = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(*type));
    if (!type)
    {
        ERR("Failed to allocate type memory.\n");
        return NULL;
    }

    type->vtbl = &d3d10_effect_type_vtbl;
    type->id = offset;
    hr = parse_fx10_type(type, data + offset, data);
    if (FAILED(hr))
    {
        ERR("Failed to parse type info, hr %#x.\n", hr);
        HeapFree(GetProcessHeap(), 0, type);
        return NULL;
    }

    if (wine_rb_put(&effect->types, &offset, &type->entry) == -1)
    {
        ERR("Failed to insert type entry.\n");
        HeapFree(GetProcessHeap(), 0, type);
        return NULL;
    }

    return type;
}

static HRESULT parse_fx10_variable(struct d3d10_effect_variable *v, const char **ptr, const char *data)
{
    DWORD offset;
    unsigned int i;

    read_dword(ptr, &offset);
    TRACE("Variable name at offset %#x.\n", offset);

    v->name = copy_name(data + offset);
    if (!v->name)
    {
        ERR("Failed to copy name.\n");
        return E_OUTOFMEMORY;
    }
    TRACE("Variable name: %s.\n", v->name);

    read_dword(ptr, &offset);
    TRACE("Variable type info at offset %#x.\n", offset);
    v->type = get_fx10_type(v->buffer->effect, data, offset);
    if (!v->type)
    {
        ERR("Failed to get variable type.\n");
        return E_FAIL;
    }

    skip_dword_unknown(ptr, 1);

    read_dword(ptr, &v->buffer_offset);
    TRACE("Variable offset in buffer: %#x.\n", v->buffer_offset);

    skip_dword_unknown(ptr, 1);

    read_dword(ptr, &v->flag);
    TRACE("Variable flag: %#x.\n", v->flag);

    read_dword(ptr, &v->annotation_count);
    for(i = 0; i < v->annotation_count; ++i)
    {
        parse_fx10_annotation(ptr);
    }

    return S_OK;
}

static HRESULT parse_fx10_local_buffer(struct d3d10_effect_local_buffer *l, const char **ptr, const char *data)
{
    unsigned int i;
    DWORD offset;

    read_dword(ptr, &offset);
    TRACE("Local buffer name at offset %#x.\n", offset);

    l->name = copy_name(data + offset);
    if (!l->name)
    {
        ERR("Failed to copy name.\n");
        return E_OUTOFMEMORY;
    }
    TRACE("Local buffer name: %s.\n", l->name);

    read_dword(ptr, &l->data_size);
    TRACE("Local buffer data size: %#x.\n", l->data_size);

    skip_dword_unknown(ptr, 1);

    read_dword(ptr, &l->variable_count);
    TRACE("Local buffer variable count: %#x.\n", l->variable_count);

    skip_dword_unknown(ptr, 1);

    read_dword(ptr, &l->annotation_count);
    for(i = 0; i < l->annotation_count; ++i)
    {
        parse_fx10_annotation(ptr);
    }

    l->variables = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, l->variable_count * sizeof(*l->variables));
    if (!l->variables)
    {
        ERR("Failed to allocate variables memory.\n");
        return E_OUTOFMEMORY;
    }

    for (i = 0; i < l->variable_count; ++i)
    {
        struct d3d10_effect_variable *v = &l->variables[i];
        HRESULT hr;

        v->vtbl = &d3d10_effect_variable_vtbl;
        v->buffer = l;

        hr = parse_fx10_variable(v, ptr, data);
        if (FAILED(hr)) return hr;
    }

    return S_OK;
}

static int d3d10_effect_type_compare(const void *key, const struct wine_rb_entry *entry)
{
    const struct d3d10_effect_type *t = WINE_RB_ENTRY_VALUE(entry, const struct d3d10_effect_type, entry);
    const DWORD *id = key;

    return *id - t->id;
}

static void d3d10_effect_type_destroy(struct wine_rb_entry *entry, void *context)
{
    struct d3d10_effect_type *t = WINE_RB_ENTRY_VALUE(entry, struct d3d10_effect_type, entry);

    TRACE("effect type %p.\n", t);

    HeapFree(GetProcessHeap(), 0, t->name);
    HeapFree(GetProcessHeap(), 0, t);
}

static const struct wine_rb_functions d3d10_effect_type_rb_functions =
{
    d3d10_rb_alloc,
    d3d10_rb_realloc,
    d3d10_rb_free,
    d3d10_effect_type_compare,
};

static HRESULT parse_fx10_body(struct d3d10_effect *e, const char *data, DWORD data_size)
{
    const char *ptr = data + e->index_offset;
    unsigned int i;
    HRESULT hr;

    if (wine_rb_init(&e->types, &d3d10_effect_type_rb_functions) == -1)
    {
        ERR("Failed to initialize type rbtree.\n");
        return E_FAIL;
    }

    e->local_buffers = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, e->local_buffer_count * sizeof(*e->local_buffers));
    if (!e->local_buffers)
    {
        ERR("Failed to allocate local buffer memory.\n");
        return E_OUTOFMEMORY;
    }

    e->techniques = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, e->technique_count * sizeof(*e->techniques));
    if (!e->techniques)
    {
        ERR("Failed to allocate techniques memory\n");
        return E_OUTOFMEMORY;
    }

    for (i = 0; i < e->local_buffer_count; ++i)
    {
        struct d3d10_effect_local_buffer *l = &e->local_buffers[i];
        l->vtbl = &d3d10_effect_constant_buffer_vtbl;
        l->effect = e;

        hr = parse_fx10_local_buffer(l, &ptr, data);
        if (FAILED(hr)) return hr;
    }

    for (i = 0; i < e->technique_count; ++i)
    {
        struct d3d10_effect_technique *t = &e->techniques[i];

        t->vtbl = &d3d10_effect_technique_vtbl;
        t->effect = e;

        hr = parse_fx10_technique(t, &ptr, data);
        if (FAILED(hr)) return hr;
    }

    return S_OK;
}

static HRESULT parse_fx10(struct d3d10_effect *e, const char *data, DWORD data_size)
{
    const char *ptr = data;
    DWORD unknown;

    /* Compiled target version (e.g. fx_4_0=0xfeff1001, fx_4_1=0xfeff1011). */
    read_dword(&ptr, &e->version);
    TRACE("Target: %#x\n", e->version);

    read_dword(&ptr, &e->local_buffer_count);
    TRACE("Local buffer count: %u.\n", e->local_buffer_count);

    read_dword(&ptr, &e->variable_count);
    TRACE("Variable count: %u\n", e->variable_count);

    read_dword(&ptr, &e->object_count);
    TRACE("Object count: %u\n", e->object_count);

    read_dword(&ptr, &e->sharedbuffers_count);
    TRACE("Sharedbuffers count: %u\n", e->sharedbuffers_count);

    /* Number of variables in shared buffers? */
    read_dword(&ptr, &unknown);
    FIXME("Unknown 0: %u\n", unknown);

    read_dword(&ptr, &e->sharedobjects_count);
    TRACE("Sharedobjects count: %u\n", e->sharedobjects_count);

    read_dword(&ptr, &e->technique_count);
    TRACE("Technique count: %u\n", e->technique_count);

    read_dword(&ptr, &e->index_offset);
    TRACE("Index offset: %#x\n", e->index_offset);

    read_dword(&ptr, &unknown);
    FIXME("Unknown 1: %u\n", unknown);

    read_dword(&ptr, &e->texture_count);
    TRACE("Texture count: %u\n", e->texture_count);

    read_dword(&ptr, &e->dephstencilstate_count);
    TRACE("Depthstencilstate count: %u\n", e->dephstencilstate_count);

    read_dword(&ptr, &e->blendstate_count);
    TRACE("Blendstate count: %u\n", e->blendstate_count);

    read_dword(&ptr, &e->rasterizerstate_count);
    TRACE("Rasterizerstate count: %u\n", e->rasterizerstate_count);

    read_dword(&ptr, &e->samplerstate_count);
    TRACE("Samplerstate count: %u\n", e->samplerstate_count);

    read_dword(&ptr, &e->rendertargetview_count);
    TRACE("Rendertargetview count: %u\n", e->rendertargetview_count);

    read_dword(&ptr, &e->depthstencilview_count);
    TRACE("Depthstencilview count: %u\n", e->depthstencilview_count);

    read_dword(&ptr, &e->shader_call_count);
    TRACE("Shader call count: %u\n", e->shader_call_count);

    read_dword(&ptr, &e->shader_compile_count);
    TRACE("Shader compile count: %u\n", e->shader_compile_count);

    return parse_fx10_body(e, ptr, data_size - (ptr - data));
}

static HRESULT fx10_chunk_handler(const char *data, DWORD data_size, DWORD tag, void *ctx)
{
    struct d3d10_effect *e = ctx;
    char tag_str[5];

    memcpy(tag_str, &tag, 4);
    tag_str[4] = '\0';
    TRACE("tag: %s\n", tag_str);

    TRACE("chunk size: %#x\n", data_size);

    switch(tag)
    {
        case TAG_FX10:
            return parse_fx10(e, data, data_size);

        default:
            FIXME("Unhandled chunk %s\n", tag_str);
            return S_OK;
    }
}

HRESULT d3d10_effect_parse(struct d3d10_effect *This, const void *data, SIZE_T data_size)
{
    return parse_dxbc(data, data_size, fx10_chunk_handler, This);
}

static void d3d10_effect_object_destroy(struct d3d10_effect_object *o)
{
    TRACE("effect object %p.\n", o);

    switch(o->type)
    {
        case D3D10_EOT_VERTEXSHADER:
        case D3D10_EOT_PIXELSHADER:
        case D3D10_EOT_GEOMETRYSHADER:
            HeapFree(GetProcessHeap(), 0, ((struct d3d10_effect_shader_variable *)o->data)->input_signature);
            break;

        default:
            break;
    }
    HeapFree(GetProcessHeap(), 0, o->data);
}

static HRESULT d3d10_effect_object_apply(struct d3d10_effect_object *o)
{
    ID3D10Device *device = o->pass->technique->effect->device;

    TRACE("effect object %p, type %#x.\n", o, o->type);

    switch(o->type)
    {
        case D3D10_EOT_VERTEXSHADER:
            ID3D10Device_VSSetShader(device, ((struct d3d10_effect_shader_variable *)o->data)->shader.vs);
            return S_OK;

        case D3D10_EOT_PIXELSHADER:
            ID3D10Device_PSSetShader(device, ((struct d3d10_effect_shader_variable *)o->data)->shader.ps);
            return S_OK;

        case D3D10_EOT_GEOMETRYSHADER:
            ID3D10Device_GSSetShader(device, ((struct d3d10_effect_shader_variable *)o->data)->shader.gs);
            return S_OK;

        default:
            FIXME("Unhandled effect object type %#x.\n", o->type);
            return E_FAIL;
    }
}

static void d3d10_effect_pass_destroy(struct d3d10_effect_pass *p)
{
    TRACE("pass %p\n", p);

    HeapFree(GetProcessHeap(), 0, p->name);
    if (p->objects)
    {
        unsigned int i;
        for (i = 0; i < p->object_count; ++i)
        {
            d3d10_effect_object_destroy(&p->objects[i]);
        }
        HeapFree(GetProcessHeap(), 0, p->objects);
    }
}

static void d3d10_effect_technique_destroy(struct d3d10_effect_technique *t)
{
    TRACE("technique %p\n", t);

    HeapFree(GetProcessHeap(), 0, t->name);
    if (t->passes)
    {
        unsigned int i;
        for (i = 0; i < t->pass_count; ++i)
        {
            d3d10_effect_pass_destroy(&t->passes[i]);
        }
        HeapFree(GetProcessHeap(), 0, t->passes);
    }
}

static void d3d10_effect_variable_destroy(struct d3d10_effect_variable *v)
{
    TRACE("variable %p.\n", v);

    HeapFree(GetProcessHeap(), 0, v->name);
}

static void d3d10_effect_local_buffer_destroy(struct d3d10_effect_local_buffer *l)
{
    TRACE("local buffer %p.\n", l);

    HeapFree(GetProcessHeap(), 0, l->name);
    if (l->variables)
    {
        unsigned int i;
        for (i = 0; i < l->variable_count; ++i)
        {
            d3d10_effect_variable_destroy(&l->variables[i]);
        }
        HeapFree(GetProcessHeap(), 0, l->variables);
    }
}

/* IUnknown methods */

static HRESULT STDMETHODCALLTYPE d3d10_effect_QueryInterface(ID3D10Effect *iface, REFIID riid, void **object)
{
    TRACE("iface %p, riid %s, object %p\n", iface, debugstr_guid(riid), object);

    if (IsEqualGUID(riid, &IID_ID3D10Effect)
            || IsEqualGUID(riid, &IID_IUnknown))
    {
        IUnknown_AddRef(iface);
        *object = iface;
        return S_OK;
    }

    WARN("%s not implemented, returning E_NOINTERFACE\n", debugstr_guid(riid));

    *object = NULL;
    return E_NOINTERFACE;
}

static ULONG STDMETHODCALLTYPE d3d10_effect_AddRef(ID3D10Effect *iface)
{
    struct d3d10_effect *This = (struct d3d10_effect *)iface;
    ULONG refcount = InterlockedIncrement(&This->refcount);

    TRACE("%p increasing refcount to %u\n", This, refcount);

    return refcount;
}

static ULONG STDMETHODCALLTYPE d3d10_effect_Release(ID3D10Effect *iface)
{
    struct d3d10_effect *This = (struct d3d10_effect *)iface;
    ULONG refcount = InterlockedDecrement(&This->refcount);

    TRACE("%p decreasing refcount to %u\n", This, refcount);

    if (!refcount)
    {
        unsigned int i;

        if (This->techniques)
        {
            for (i = 0; i < This->technique_count; ++i)
            {
                d3d10_effect_technique_destroy(&This->techniques[i]);
            }
            HeapFree(GetProcessHeap(), 0, This->techniques);
        }

        if (This->local_buffers)
        {
            for (i = 0; i < This->local_buffer_count; ++i)
            {
                d3d10_effect_local_buffer_destroy(&This->local_buffers[i]);
            }
            HeapFree(GetProcessHeap(), 0, This->local_buffers);
        }

        wine_rb_destroy(&This->types, d3d10_effect_type_destroy, NULL);

        ID3D10Device_Release(This->device);
        HeapFree(GetProcessHeap(), 0, This);
    }

    return refcount;
}

/* ID3D10Effect methods */

static BOOL STDMETHODCALLTYPE d3d10_effect_IsValid(ID3D10Effect *iface)
{
    FIXME("iface %p stub!\n", iface);

    return FALSE;
}

static BOOL STDMETHODCALLTYPE d3d10_effect_IsPool(ID3D10Effect *iface)
{
    FIXME("iface %p stub!\n", iface);

    return FALSE;
}

static HRESULT STDMETHODCALLTYPE d3d10_effect_GetDevice(ID3D10Effect *iface, ID3D10Device **device)
{
    struct d3d10_effect *This = (struct d3d10_effect *)iface;

    TRACE("iface %p, device %p\n", iface, device);

    ID3D10Device_AddRef(This->device);
    *device = This->device;

    return S_OK;
}

static HRESULT STDMETHODCALLTYPE d3d10_effect_GetDesc(ID3D10Effect *iface, D3D10_EFFECT_DESC *desc)
{
    FIXME("iface %p, desc %p stub!\n", iface, desc);

    return E_NOTIMPL;
}

static struct ID3D10EffectConstantBuffer * STDMETHODCALLTYPE d3d10_effect_GetConstantBufferByIndex(ID3D10Effect *iface,
        UINT index)
{
    struct d3d10_effect *This = (struct d3d10_effect *)iface;
    struct d3d10_effect_local_buffer *l;

    TRACE("iface %p, index %u\n", iface, index);

    if (index >= This->local_buffer_count)
    {
        WARN("Invalid index specified\n");
        return (ID3D10EffectConstantBuffer *)&null_local_buffer;
    }

    l = &This->local_buffers[index];

    TRACE("Returning buffer %p, \"%s\"\n", l, l->name);

    return (ID3D10EffectConstantBuffer *)l;
}

static struct ID3D10EffectConstantBuffer * STDMETHODCALLTYPE d3d10_effect_GetConstantBufferByName(ID3D10Effect *iface,
        LPCSTR name)
{
    struct d3d10_effect *This = (struct d3d10_effect *)iface;
    unsigned int i;

    TRACE("iface %p, name \"%s\"\n", iface, name);

    for (i = 0; i < This->local_buffer_count; ++i)
    {
        struct d3d10_effect_local_buffer *l = &This->local_buffers[i];

        if (!strcmp(l->name, name))
        {
            TRACE("Returning buffer %p.\n", l);
            return (ID3D10EffectConstantBuffer *)l;
        }
    }

    WARN("Invalid name specified\n");

    return (ID3D10EffectConstantBuffer *)&null_local_buffer;
}

static struct ID3D10EffectVariable * STDMETHODCALLTYPE d3d10_effect_GetVariableByIndex(ID3D10Effect *iface, UINT index)
{
    FIXME("iface %p, index %u stub!\n", iface, index);

    return NULL;
}

static struct ID3D10EffectVariable * STDMETHODCALLTYPE d3d10_effect_GetVariableByName(ID3D10Effect *iface, LPCSTR name)
{
    struct d3d10_effect *This = (struct d3d10_effect *)iface;
    unsigned int i;

    TRACE("iface %p, name \"%s\"\n", iface, name);

    for (i = 0; i < This->local_buffer_count; ++i)
    {
        struct d3d10_effect_local_buffer *l = &This->local_buffers[i];
        unsigned int j;

        for (j = 0; j < l->variable_count; ++j)
        {
            struct d3d10_effect_variable *v = &l->variables[j];

            if (!strcmp(v->name, name))
            {
                TRACE("Returning variable %p.\n", v);
                return (ID3D10EffectVariable *)v;
            }
        }
    }

    WARN("Invalid name specified\n");

    return (ID3D10EffectVariable *)&null_variable;
}

static struct ID3D10EffectVariable * STDMETHODCALLTYPE d3d10_effect_GetVariableBySemantic(ID3D10Effect *iface,
        LPCSTR semantic)
{
    FIXME("iface %p, semantic \"%s\" stub!\n", iface, semantic);

    return NULL;
}

static struct ID3D10EffectTechnique * STDMETHODCALLTYPE d3d10_effect_GetTechniqueByIndex(ID3D10Effect *iface,
        UINT index)
{
    struct d3d10_effect *This = (struct d3d10_effect *)iface;
    struct d3d10_effect_technique *t;

    TRACE("iface %p, index %u\n", iface, index);

    if (index >= This->technique_count)
    {
        WARN("Invalid index specified\n");
        return (ID3D10EffectTechnique *)&null_technique;
    }

    t = &This->techniques[index];

    TRACE("Returning technique %p, \"%s\"\n", t, t->name);

    return (ID3D10EffectTechnique *)t;
}

static struct ID3D10EffectTechnique * STDMETHODCALLTYPE d3d10_effect_GetTechniqueByName(ID3D10Effect *iface,
        LPCSTR name)
{
    struct d3d10_effect *This = (struct d3d10_effect *)iface;
    unsigned int i;

    TRACE("iface %p, name \"%s\"\n", iface, name);

    for (i = 0; i < This->technique_count; ++i)
    {
        struct d3d10_effect_technique *t = &This->techniques[i];
        if (!strcmp(t->name, name))
        {
            TRACE("Returning technique %p\n", t);
            return (ID3D10EffectTechnique *)t;
        }
    }

    WARN("Invalid name specified\n");

    return (ID3D10EffectTechnique *)&null_technique;
}

static HRESULT STDMETHODCALLTYPE d3d10_effect_Optimize(ID3D10Effect *iface)
{
    FIXME("iface %p stub!\n", iface);

    return E_NOTIMPL;
}

static BOOL STDMETHODCALLTYPE d3d10_effect_IsOptimized(ID3D10Effect *iface)
{
    FIXME("iface %p stub!\n", iface);

    return FALSE;
}

const struct ID3D10EffectVtbl d3d10_effect_vtbl =
{
    /* IUnknown methods */
    d3d10_effect_QueryInterface,
    d3d10_effect_AddRef,
    d3d10_effect_Release,
    /* ID3D10Effect methods */
    d3d10_effect_IsValid,
    d3d10_effect_IsPool,
    d3d10_effect_GetDevice,
    d3d10_effect_GetDesc,
    d3d10_effect_GetConstantBufferByIndex,
    d3d10_effect_GetConstantBufferByName,
    d3d10_effect_GetVariableByIndex,
    d3d10_effect_GetVariableByName,
    d3d10_effect_GetVariableBySemantic,
    d3d10_effect_GetTechniqueByIndex,
    d3d10_effect_GetTechniqueByName,
    d3d10_effect_Optimize,
    d3d10_effect_IsOptimized,
};

/* ID3D10EffectTechnique methods */

static BOOL STDMETHODCALLTYPE d3d10_effect_technique_IsValid(ID3D10EffectTechnique *iface)
{
    TRACE("iface %p\n", iface);

    return (struct d3d10_effect_technique *)iface != &null_technique;
}

static HRESULT STDMETHODCALLTYPE d3d10_effect_technique_GetDesc(ID3D10EffectTechnique *iface,
        D3D10_TECHNIQUE_DESC *desc)
{
    struct d3d10_effect_technique *This = (struct d3d10_effect_technique *)iface;

    TRACE("iface %p, desc %p\n", iface, desc);

    if(This == &null_technique)
    {
        WARN("Null technique specified\n");
        return E_FAIL;
    }

    if(!desc)
    {
        WARN("Invalid argument specified\n");
        return E_INVALIDARG;
    }

    desc->Name = This->name;
    desc->Passes = This->pass_count;
    WARN("Annotations not implemented\n");
    desc->Annotations = 0;

    return S_OK;
}

static struct ID3D10EffectVariable * STDMETHODCALLTYPE d3d10_effect_technique_GetAnnotationByIndex(
        ID3D10EffectTechnique *iface, UINT index)
{
    FIXME("iface %p, index %u stub!\n", iface, index);

    return NULL;
}

static struct ID3D10EffectVariable * STDMETHODCALLTYPE d3d10_effect_technique_GetAnnotationByName(
        ID3D10EffectTechnique *iface, LPCSTR name)
{
    FIXME("iface %p, name \"%s\" stub!\n", iface, name);

    return NULL;
}

static struct ID3D10EffectPass * STDMETHODCALLTYPE d3d10_effect_technique_GetPassByIndex(ID3D10EffectTechnique *iface,
        UINT index)
{
    struct d3d10_effect_technique *This = (struct d3d10_effect_technique *)iface;
    struct d3d10_effect_pass *p;

    TRACE("iface %p, index %u\n", iface, index);

    if (index >= This->pass_count)
    {
        WARN("Invalid index specified\n");
        return (ID3D10EffectPass *)&null_pass;
    }

    p = &This->passes[index];

    TRACE("Returning pass %p, \"%s\"\n", p, p->name);

    return (ID3D10EffectPass *)p;
}

static struct ID3D10EffectPass * STDMETHODCALLTYPE d3d10_effect_technique_GetPassByName(ID3D10EffectTechnique *iface,
        LPCSTR name)
{
    struct d3d10_effect_technique *This = (struct d3d10_effect_technique *)iface;
    unsigned int i;

    TRACE("iface %p, name \"%s\"\n", iface, name);

    for (i = 0; i < This->pass_count; ++i)
    {
        struct d3d10_effect_pass *p = &This->passes[i];
        if (!strcmp(p->name, name))
        {
            TRACE("Returning pass %p\n", p);
            return (ID3D10EffectPass *)p;
        }
    }

    WARN("Invalid name specified\n");

    return (ID3D10EffectPass *)&null_pass;
}

static HRESULT STDMETHODCALLTYPE d3d10_effect_technique_ComputeStateBlockMask(ID3D10EffectTechnique *iface,
        D3D10_STATE_BLOCK_MASK *mask)
{
    FIXME("iface %p,mask %p stub!\n", iface, mask);

    return E_NOTIMPL;
}

static const struct ID3D10EffectTechniqueVtbl d3d10_effect_technique_vtbl =
{
    /* ID3D10EffectTechnique methods */
    d3d10_effect_technique_IsValid,
    d3d10_effect_technique_GetDesc,
    d3d10_effect_technique_GetAnnotationByIndex,
    d3d10_effect_technique_GetAnnotationByName,
    d3d10_effect_technique_GetPassByIndex,
    d3d10_effect_technique_GetPassByName,
    d3d10_effect_technique_ComputeStateBlockMask,
};

/* ID3D10EffectPass methods */

static BOOL STDMETHODCALLTYPE d3d10_effect_pass_IsValid(ID3D10EffectPass *iface)
{
    TRACE("iface %p\n", iface);

    return (struct d3d10_effect_pass *)iface != &null_pass;
}

static HRESULT STDMETHODCALLTYPE d3d10_effect_pass_GetDesc(ID3D10EffectPass *iface, D3D10_PASS_DESC *desc)
{
    struct d3d10_effect_pass *This = (struct d3d10_effect_pass *)iface;
    unsigned int i;

    FIXME("iface %p, desc %p partial stub!\n", iface, desc);

    if(This == &null_pass)
    {
        WARN("Null pass specified\n");
        return E_FAIL;
    }

    if(!desc)
    {
        WARN("Invalid argument specified\n");
        return E_INVALIDARG;
    }

    memset(desc, 0, sizeof(*desc));
    desc->Name = This->name;
    for (i = 0; i < This->object_count; ++i)
    {
        struct d3d10_effect_object *o = &This->objects[i];
        if (o->type == D3D10_EOT_VERTEXSHADER)
        {
            struct d3d10_effect_shader_variable *s = o->data;
            desc->pIAInputSignature = (BYTE *)s->input_signature;
            desc->IAInputSignatureSize = s->input_signature_size;
            break;
        }
    }

    return S_OK;
}

static HRESULT STDMETHODCALLTYPE d3d10_effect_pass_GetVertexShaderDesc(ID3D10EffectPass *iface,
        D3D10_PASS_SHADER_DESC *desc)
{
    FIXME("iface %p, desc %p stub!\n", iface, desc);

    return E_NOTIMPL;
}

static HRESULT STDMETHODCALLTYPE d3d10_effect_pass_GetGeometryShaderDesc(ID3D10EffectPass *iface,
        D3D10_PASS_SHADER_DESC *desc)
{
    FIXME("iface %p, desc %p stub!\n", iface, desc);

    return E_NOTIMPL;
}

static HRESULT STDMETHODCALLTYPE d3d10_effect_pass_GetPixelShaderDesc(ID3D10EffectPass *iface,
        D3D10_PASS_SHADER_DESC *desc)
{
    FIXME("iface %p, desc %p stub!\n", iface, desc);

    return E_NOTIMPL;
}

static struct ID3D10EffectVariable * STDMETHODCALLTYPE d3d10_effect_pass_GetAnnotationByIndex(ID3D10EffectPass *iface,
        UINT index)
{
    FIXME("iface %p, index %u stub!\n", iface, index);

    return NULL;
}

static struct ID3D10EffectVariable * STDMETHODCALLTYPE d3d10_effect_pass_GetAnnotationByName(ID3D10EffectPass *iface,
        LPCSTR name)
{
    FIXME("iface %p, name \"%s\" stub!\n", iface, name);

    return NULL;
}

static HRESULT STDMETHODCALLTYPE d3d10_effect_pass_Apply(ID3D10EffectPass *iface, UINT flags)
{
    struct d3d10_effect_pass *This = (struct d3d10_effect_pass *)iface;
    HRESULT hr = S_OK;
    unsigned int i;

    TRACE("iface %p, flags %#x\n", iface, flags);

    if (flags) FIXME("Ignoring flags (%#x)\n", flags);

    for (i = 0; i < This->object_count; ++i)
    {
        hr = d3d10_effect_object_apply(&This->objects[i]);
        if (FAILED(hr)) break;
    }

    return hr;
}

static HRESULT STDMETHODCALLTYPE d3d10_effect_pass_ComputeStateBlockMask(ID3D10EffectPass *iface,
        D3D10_STATE_BLOCK_MASK *mask)
{
    FIXME("iface %p, mask %p stub!\n", iface, mask);

    return E_NOTIMPL;
}

static const struct ID3D10EffectPassVtbl d3d10_effect_pass_vtbl =
{
    /* ID3D10EffectPass methods */
    d3d10_effect_pass_IsValid,
    d3d10_effect_pass_GetDesc,
    d3d10_effect_pass_GetVertexShaderDesc,
    d3d10_effect_pass_GetGeometryShaderDesc,
    d3d10_effect_pass_GetPixelShaderDesc,
    d3d10_effect_pass_GetAnnotationByIndex,
    d3d10_effect_pass_GetAnnotationByName,
    d3d10_effect_pass_Apply,
    d3d10_effect_pass_ComputeStateBlockMask,
};

/* ID3D10EffectVariable methods */

static BOOL STDMETHODCALLTYPE d3d10_effect_variable_IsValid(ID3D10EffectVariable *iface)
{
    TRACE("iface %p\n", iface);

    return (struct d3d10_effect_variable *)iface != &null_variable;
}

static struct ID3D10EffectType * STDMETHODCALLTYPE d3d10_effect_variable_GetType(ID3D10EffectVariable *iface)
{
    struct d3d10_effect_variable *This = (struct d3d10_effect_variable *)iface;

    TRACE("iface %p\n", iface);

    return (ID3D10EffectType *)This->type;
}

static HRESULT STDMETHODCALLTYPE d3d10_effect_variable_GetDesc(ID3D10EffectVariable *iface,
        D3D10_EFFECT_VARIABLE_DESC *desc)
{
    FIXME("iface %p, desc %p stub!\n", iface, desc);

    return E_NOTIMPL;
}

static struct ID3D10EffectVariable * STDMETHODCALLTYPE d3d10_effect_variable_GetAnnotationByIndex(
        ID3D10EffectVariable *iface, UINT index)
{
    FIXME("iface %p, index %u stub!\n", iface, index);

    return NULL;
}

static struct ID3D10EffectVariable * STDMETHODCALLTYPE d3d10_effect_variable_GetAnnotationByName(
        ID3D10EffectVariable *iface, LPCSTR name)
{
    FIXME("iface %p, name \"%s\" stub!\n", iface, name);

    return NULL;
}

static struct ID3D10EffectVariable * STDMETHODCALLTYPE d3d10_effect_variable_GetMemberByIndex(
        ID3D10EffectVariable *iface, UINT index)
{
    FIXME("iface %p, index %u stub!\n", iface, index);

    return NULL;
}

static struct ID3D10EffectVariable * STDMETHODCALLTYPE d3d10_effect_variable_GetMemberByName(
        ID3D10EffectVariable *iface, LPCSTR name)
{
    FIXME("iface %p, name \"%s\" stub!\n", iface, name);

    return NULL;
}

static struct ID3D10EffectVariable * STDMETHODCALLTYPE d3d10_effect_variable_GetMemberBySemantic(
        ID3D10EffectVariable *iface, LPCSTR semantic)
{
    FIXME("iface %p, semantic \"%s\" stub!\n", iface, semantic);

    return NULL;
}

static struct ID3D10EffectVariable * STDMETHODCALLTYPE d3d10_effect_variable_GetElement(
        ID3D10EffectVariable *iface, UINT index)
{
    FIXME("iface %p, index %u stub!\n", iface, index);

    return NULL;
}

static struct ID3D10EffectConstantBuffer * STDMETHODCALLTYPE d3d10_effect_variable_GetParentConstantBuffer(
        ID3D10EffectVariable *iface)
{
    FIXME("iface %p stub!\n", iface);

    return NULL;
}

static struct ID3D10EffectScalarVariable * STDMETHODCALLTYPE d3d10_effect_variable_AsScalar(
        ID3D10EffectVariable *iface)
{
    FIXME("iface %p stub!\n", iface);

    return NULL;
}

static struct ID3D10EffectVectorVariable * STDMETHODCALLTYPE d3d10_effect_variable_AsVector(
        ID3D10EffectVariable *iface)
{
    FIXME("iface %p stub!\n", iface);

    return NULL;
}

static struct ID3D10EffectMatrixVariable * STDMETHODCALLTYPE d3d10_effect_variable_AsMatrix(
        ID3D10EffectVariable *iface)
{
    FIXME("iface %p stub!\n", iface);

    return NULL;
}

static struct ID3D10EffectStringVariable * STDMETHODCALLTYPE d3d10_effect_variable_AsString(
        ID3D10EffectVariable *iface)
{
    FIXME("iface %p stub!\n", iface);

    return NULL;
}

static struct ID3D10EffectShaderResourceVariable * STDMETHODCALLTYPE d3d10_effect_variable_AsShaderResource(
        ID3D10EffectVariable *iface)
{
    FIXME("iface %p stub!\n", iface);

    return NULL;
}

static struct ID3D10EffectRenderTargetViewVariable * STDMETHODCALLTYPE d3d10_effect_variable_AsRenderTargetView(
        ID3D10EffectVariable *iface)
{
    FIXME("iface %p stub!\n", iface);

    return NULL;
}

static struct ID3D10EffectDepthStencilViewVariable * STDMETHODCALLTYPE d3d10_effect_variable_AsDepthStencilView(
        ID3D10EffectVariable *iface)
{
    FIXME("iface %p stub!\n", iface);

    return NULL;
}

static struct ID3D10EffectConstantBuffer * STDMETHODCALLTYPE d3d10_effect_variable_AsConstantBuffer(
        ID3D10EffectVariable *iface)
{
    FIXME("iface %p stub!\n", iface);

    return NULL;
}

static struct ID3D10EffectShaderVariable * STDMETHODCALLTYPE d3d10_effect_variable_AsShader(
        ID3D10EffectVariable *iface)
{
    FIXME("iface %p stub!\n", iface);

    return NULL;
}

static struct ID3D10EffectBlendVariable * STDMETHODCALLTYPE d3d10_effect_variable_AsBlend(ID3D10EffectVariable *iface)
{
    FIXME("iface %p stub!\n", iface);

    return NULL;
}

static struct ID3D10EffectDepthStencilVariable * STDMETHODCALLTYPE d3d10_effect_variable_AsDepthStencil(
        ID3D10EffectVariable *iface)
{
    FIXME("iface %p stub!\n", iface);

    return NULL;
}

static struct ID3D10EffectRasterizerVariable * STDMETHODCALLTYPE d3d10_effect_variable_AsRasterizer(
        ID3D10EffectVariable *iface)
{
    FIXME("iface %p stub!\n", iface);

    return NULL;
}

static struct ID3D10EffectSamplerVariable * STDMETHODCALLTYPE d3d10_effect_variable_AsSampler(
        ID3D10EffectVariable *iface)
{
    FIXME("iface %p stub!\n", iface);

    return NULL;
}

static HRESULT STDMETHODCALLTYPE d3d10_effect_variable_SetRawValue(ID3D10EffectVariable *iface,
        void *data, UINT offset, UINT count)
{
    FIXME("iface %p, data %p, offset %u, count %u stub!\n", iface, data, offset, count);

    return E_NOTIMPL;
}

static HRESULT STDMETHODCALLTYPE d3d10_effect_variable_GetRawValue(ID3D10EffectVariable *iface,
        void *data, UINT offset, UINT count)
{
    FIXME("iface %p, data %p, offset %u, count %u stub!\n", iface, data, offset, count);

    return E_NOTIMPL;
}

static const struct ID3D10EffectVariableVtbl d3d10_effect_variable_vtbl =
{
    /* ID3D10EffectVariable methods */
    d3d10_effect_variable_IsValid,
    d3d10_effect_variable_GetType,
    d3d10_effect_variable_GetDesc,
    d3d10_effect_variable_GetAnnotationByIndex,
    d3d10_effect_variable_GetAnnotationByName,
    d3d10_effect_variable_GetMemberByIndex,
    d3d10_effect_variable_GetMemberByName,
    d3d10_effect_variable_GetMemberBySemantic,
    d3d10_effect_variable_GetElement,
    d3d10_effect_variable_GetParentConstantBuffer,
    d3d10_effect_variable_AsScalar,
    d3d10_effect_variable_AsVector,
    d3d10_effect_variable_AsMatrix,
    d3d10_effect_variable_AsString,
    d3d10_effect_variable_AsShaderResource,
    d3d10_effect_variable_AsRenderTargetView,
    d3d10_effect_variable_AsDepthStencilView,
    d3d10_effect_variable_AsConstantBuffer,
    d3d10_effect_variable_AsShader,
    d3d10_effect_variable_AsBlend,
    d3d10_effect_variable_AsDepthStencil,
    d3d10_effect_variable_AsRasterizer,
    d3d10_effect_variable_AsSampler,
    d3d10_effect_variable_SetRawValue,
    d3d10_effect_variable_GetRawValue,
};

/* ID3D10EffectVariable methods */
static BOOL STDMETHODCALLTYPE d3d10_effect_constant_buffer_IsValid(ID3D10EffectConstantBuffer *iface)
{
    TRACE("iface %p\n", iface);

    return (struct d3d10_effect_local_buffer *)iface != &null_local_buffer;
}

static struct ID3D10EffectType * STDMETHODCALLTYPE d3d10_effect_constant_buffer_GetType(ID3D10EffectConstantBuffer *iface)
{
    FIXME("iface %p stub!\n", iface);

    return NULL;
}

static HRESULT STDMETHODCALLTYPE d3d10_effect_constant_buffer_GetDesc(ID3D10EffectConstantBuffer *iface,
        D3D10_EFFECT_VARIABLE_DESC *desc)
{
    FIXME("iface %p, desc %p stub!\n", iface, desc);

    return E_NOTIMPL;
}

static struct ID3D10EffectVariable * STDMETHODCALLTYPE d3d10_effect_constant_buffer_GetAnnotationByIndex(
        ID3D10EffectConstantBuffer *iface, UINT index)
{
    FIXME("iface %p, index %u stub!\n", iface, index);

    return NULL;
}

static struct ID3D10EffectVariable * STDMETHODCALLTYPE d3d10_effect_constant_buffer_GetAnnotationByName(
        ID3D10EffectConstantBuffer *iface, LPCSTR name)
{
    FIXME("iface %p, name \"%s\" stub!\n", iface, name);

    return NULL;
}

static struct ID3D10EffectVariable * STDMETHODCALLTYPE d3d10_effect_constant_buffer_GetMemberByIndex(
        ID3D10EffectConstantBuffer *iface, UINT index)
{
    FIXME("iface %p, index %u stub!\n", iface, index);

    return NULL;
}

static struct ID3D10EffectVariable * STDMETHODCALLTYPE d3d10_effect_constant_buffer_GetMemberByName(
        ID3D10EffectConstantBuffer *iface, LPCSTR name)
{
    FIXME("iface %p, name \"%s\" stub!\n", iface, name);

    return NULL;
}

static struct ID3D10EffectVariable * STDMETHODCALLTYPE d3d10_effect_constant_buffer_GetMemberBySemantic(
        ID3D10EffectConstantBuffer *iface, LPCSTR semantic)
{
    FIXME("iface %p, semantic \"%s\" stub!\n", iface, semantic);

    return NULL;
}

static struct ID3D10EffectVariable * STDMETHODCALLTYPE d3d10_effect_constant_buffer_GetElement(
        ID3D10EffectConstantBuffer *iface, UINT index)
{
    FIXME("iface %p, index %u stub!\n", iface, index);

    return NULL;
}

static struct ID3D10EffectConstantBuffer * STDMETHODCALLTYPE d3d10_effect_constant_buffer_GetParentConstantBuffer(
        ID3D10EffectConstantBuffer *iface)
{
    FIXME("iface %p stub!\n", iface);

    return NULL;
}

static struct ID3D10EffectScalarVariable * STDMETHODCALLTYPE d3d10_effect_constant_buffer_AsScalar(
        ID3D10EffectConstantBuffer *iface)
{
    FIXME("iface %p stub!\n", iface);

    return NULL;
}

static struct ID3D10EffectVectorVariable * STDMETHODCALLTYPE d3d10_effect_constant_buffer_AsVector(
        ID3D10EffectConstantBuffer *iface)
{
    FIXME("iface %p stub!\n", iface);

    return NULL;
}

static struct ID3D10EffectMatrixVariable * STDMETHODCALLTYPE d3d10_effect_constant_buffer_AsMatrix(
        ID3D10EffectConstantBuffer *iface)
{
    FIXME("iface %p stub!\n", iface);

    return NULL;
}

static struct ID3D10EffectStringVariable * STDMETHODCALLTYPE d3d10_effect_constant_buffer_AsString(
        ID3D10EffectConstantBuffer *iface)
{
    FIXME("iface %p stub!\n", iface);

    return NULL;
}

static struct ID3D10EffectShaderResourceVariable * STDMETHODCALLTYPE d3d10_effect_constant_buffer_AsShaderResource(
        ID3D10EffectConstantBuffer *iface)
{
    FIXME("iface %p stub!\n", iface);

    return NULL;
}

static struct ID3D10EffectRenderTargetViewVariable * STDMETHODCALLTYPE d3d10_effect_constant_buffer_AsRenderTargetView(
        ID3D10EffectConstantBuffer *iface)
{
    FIXME("iface %p stub!\n", iface);

    return NULL;
}

static struct ID3D10EffectDepthStencilViewVariable * STDMETHODCALLTYPE d3d10_effect_constant_buffer_AsDepthStencilView(
        ID3D10EffectConstantBuffer *iface)
{
    FIXME("iface %p stub!\n", iface);

    return NULL;
}

static struct ID3D10EffectConstantBuffer * STDMETHODCALLTYPE d3d10_effect_constant_buffer_AsConstantBuffer(
        ID3D10EffectConstantBuffer *iface)
{
    FIXME("iface %p stub!\n", iface);

    return NULL;
}

static struct ID3D10EffectShaderVariable * STDMETHODCALLTYPE d3d10_effect_constant_buffer_AsShader(
        ID3D10EffectConstantBuffer *iface)
{
    FIXME("iface %p stub!\n", iface);

    return NULL;
}

static struct ID3D10EffectBlendVariable * STDMETHODCALLTYPE d3d10_effect_constant_buffer_AsBlend(ID3D10EffectConstantBuffer *iface)
{
    FIXME("iface %p stub!\n", iface);

    return NULL;
}

static struct ID3D10EffectDepthStencilVariable * STDMETHODCALLTYPE d3d10_effect_constant_buffer_AsDepthStencil(
        ID3D10EffectConstantBuffer *iface)
{
    FIXME("iface %p stub!\n", iface);

    return NULL;
}

static struct ID3D10EffectRasterizerVariable * STDMETHODCALLTYPE d3d10_effect_constant_buffer_AsRasterizer(
        ID3D10EffectConstantBuffer *iface)
{
    FIXME("iface %p stub!\n", iface);

    return NULL;
}

static struct ID3D10EffectSamplerVariable * STDMETHODCALLTYPE d3d10_effect_constant_buffer_AsSampler(
        ID3D10EffectConstantBuffer *iface)
{
    FIXME("iface %p stub!\n", iface);

    return NULL;
}

static HRESULT STDMETHODCALLTYPE d3d10_effect_constant_buffer_SetRawValue(ID3D10EffectConstantBuffer *iface,
        void *data, UINT offset, UINT count)
{
    FIXME("iface %p, data %p, offset %u, count %u stub!\n", iface, data, offset, count);

    return E_NOTIMPL;
}

static HRESULT STDMETHODCALLTYPE d3d10_effect_constant_buffer_GetRawValue(ID3D10EffectConstantBuffer *iface,
        void *data, UINT offset, UINT count)
{
    FIXME("iface %p, data %p, offset %u, count %u stub!\n", iface, data, offset, count);

    return E_NOTIMPL;
}

/* ID3D10EffectConstantBuffer methods */
static HRESULT STDMETHODCALLTYPE d3d10_effect_constant_buffer_SetConstantBuffer(ID3D10EffectConstantBuffer *iface,
        ID3D10Buffer *buffer)
{
    FIXME("iface %p, buffer %p stub!\n", iface, buffer);

    return E_NOTIMPL;
}

static HRESULT STDMETHODCALLTYPE d3d10_effect_constant_buffer_GetConstantBuffer(ID3D10EffectConstantBuffer *iface,
        ID3D10Buffer **buffer)
{
    FIXME("iface %p, buffer %p stub!\n", iface, buffer);

    return E_NOTIMPL;
}

static HRESULT STDMETHODCALLTYPE d3d10_effect_constant_buffer_SetTextureBuffer(ID3D10EffectConstantBuffer *iface,
        ID3D10ShaderResourceView *view)
{
    FIXME("iface %p, view %p stub!\n", iface, view);

    return E_NOTIMPL;
}

static HRESULT STDMETHODCALLTYPE d3d10_effect_constant_buffer_GetTextureBuffer(ID3D10EffectConstantBuffer *iface,
        ID3D10ShaderResourceView **view)
{
    FIXME("iface %p, view %p stub!\n", iface, view);

    return E_NOTIMPL;
}

static const struct ID3D10EffectConstantBufferVtbl d3d10_effect_constant_buffer_vtbl =
{
    /* ID3D10EffectVariable methods */
    d3d10_effect_constant_buffer_IsValid,
    d3d10_effect_constant_buffer_GetType,
    d3d10_effect_constant_buffer_GetDesc,
    d3d10_effect_constant_buffer_GetAnnotationByIndex,
    d3d10_effect_constant_buffer_GetAnnotationByName,
    d3d10_effect_constant_buffer_GetMemberByIndex,
    d3d10_effect_constant_buffer_GetMemberByName,
    d3d10_effect_constant_buffer_GetMemberBySemantic,
    d3d10_effect_constant_buffer_GetElement,
    d3d10_effect_constant_buffer_GetParentConstantBuffer,
    d3d10_effect_constant_buffer_AsScalar,
    d3d10_effect_constant_buffer_AsVector,
    d3d10_effect_constant_buffer_AsMatrix,
    d3d10_effect_constant_buffer_AsString,
    d3d10_effect_constant_buffer_AsShaderResource,
    d3d10_effect_constant_buffer_AsRenderTargetView,
    d3d10_effect_constant_buffer_AsDepthStencilView,
    d3d10_effect_constant_buffer_AsConstantBuffer,
    d3d10_effect_constant_buffer_AsShader,
    d3d10_effect_constant_buffer_AsBlend,
    d3d10_effect_constant_buffer_AsDepthStencil,
    d3d10_effect_constant_buffer_AsRasterizer,
    d3d10_effect_constant_buffer_AsSampler,
    d3d10_effect_constant_buffer_SetRawValue,
    d3d10_effect_constant_buffer_GetRawValue,
    /* ID3D10EffectConstantBuffer methods */
    d3d10_effect_constant_buffer_SetConstantBuffer,
    d3d10_effect_constant_buffer_GetConstantBuffer,
    d3d10_effect_constant_buffer_SetTextureBuffer,
    d3d10_effect_constant_buffer_GetTextureBuffer,
};

static BOOL STDMETHODCALLTYPE d3d10_effect_type_IsValid(ID3D10EffectType *iface)
{
    FIXME("iface %p stub!\n", iface);

    return FALSE;
}

static HRESULT STDMETHODCALLTYPE d3d10_effect_type_GetDesc(ID3D10EffectType *iface, D3D10_EFFECT_TYPE_DESC *desc)
{
    struct d3d10_effect_type *This = (struct d3d10_effect_type *)iface;

    TRACE("iface %p, desc %p\n", iface, desc);

    if (!desc) return E_INVALIDARG;

    desc->TypeName = This->name;
    desc->Class = This->type_class;
    desc->Type = This->basetype;
    desc->Elements = This->element_count;
    desc->Members = This->member_count;
    desc->Rows = This->row_count;
    desc->Columns = This->column_count;
    desc->PackedSize = This->size_packed;
    desc->UnpackedSize = This->size_unpacked;
    desc->Stride = This->stride;

    return S_OK;
}

static struct ID3D10EffectType * STDMETHODCALLTYPE d3d10_effect_type_GetMemberTypeByIndex(ID3D10EffectType *iface,
        UINT index)
{
    FIXME("iface %p, index %u stub!\n", iface, index);

    return NULL;
}

static struct ID3D10EffectType * STDMETHODCALLTYPE d3d10_effect_type_GetMemberTypeByName(ID3D10EffectType *iface,
        LPCSTR name)
{
    FIXME("iface %p, name %s stub!\n", iface, debugstr_a(name));

    return NULL;
}

static struct ID3D10EffectType * STDMETHODCALLTYPE d3d10_effect_type_GetMemberTypeBySemantic(ID3D10EffectType *iface,
        LPCSTR semantic)
{
    FIXME("iface %p, semantic %s stub!\n", iface, debugstr_a(semantic));

    return NULL;
}

static LPCSTR STDMETHODCALLTYPE d3d10_effect_type_GetMemberName(ID3D10EffectType *iface, UINT index)
{
    FIXME("iface %p, index %u stub!\n", iface, index);

    return NULL;
}

static LPCSTR STDMETHODCALLTYPE d3d10_effect_type_GetMemberSemantic(ID3D10EffectType *iface, UINT index)
{
    FIXME("iface %p, index %u stub!\n", iface, index);

    return NULL;
}

static const struct ID3D10EffectTypeVtbl d3d10_effect_type_vtbl =
{
    /* ID3D10EffectType */
    d3d10_effect_type_IsValid,
    d3d10_effect_type_GetDesc,
    d3d10_effect_type_GetMemberTypeByIndex,
    d3d10_effect_type_GetMemberTypeByName,
    d3d10_effect_type_GetMemberTypeBySemantic,
    d3d10_effect_type_GetMemberName,
    d3d10_effect_type_GetMemberSemantic,
};
