#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <ctype.h>

#include "docs_generator.h"

static int
_indent_line(Eina_Strbuf *buf, int ind)
{
   int i;
   for (i = 0; i < ind; ++i)
     eina_strbuf_append_char(buf, ' ');
   return ind;
}

#define DOC_LINE_LIMIT 79
#define DOC_LINE_TEST 59
#define DOC_LINE_OVER 39

#define DOC_LIMIT(ind) ((ind > DOC_LINE_TEST) ? (ind + DOC_LINE_OVER) \
                                              : DOC_LINE_LIMIT)

static void
_generate_ref(const char *refn, Eina_Strbuf *wbuf, Eina_Bool use_legacy)
{
   const Eolian_Declaration *decl = eolian_declaration_get_by_name(refn);
   if (decl)
     {
        char *n = strdup(eolian_declaration_name_get(decl));
        char *p = n;
        while ((p = strchr(p, '.'))) *p = '_';
        eina_strbuf_append(wbuf, n);
        free(n);
        return;
     }

   /* not a plain declaration, so it must be struct/enum field or func */
   const char *sfx = strrchr(refn, '.');
   if (!sfx) goto noref;

   Eina_Stringshare *bname = eina_stringshare_add_length(refn, sfx - refn);

   const Eolian_Type *tp = eolian_type_struct_get_by_name(bname);
   if (tp)
     {
        if (!eolian_type_struct_field_get(tp, sfx + 1))
          {
             eina_stringshare_del(bname);
             goto noref;
          }
        _generate_ref(bname, wbuf, use_legacy);
        eina_strbuf_append(wbuf, sfx);
        eina_stringshare_del(bname);
        return;
     }

   tp = eolian_type_enum_get_by_name(bname);
   if (tp)
     {
        const Eolian_Enum_Type_Field *efl = eolian_type_enum_field_get(tp, sfx + 1);
        if (!efl)
          {
             eina_stringshare_del(bname);
             goto noref;
          }
        _generate_ref(bname, wbuf, use_legacy);
        Eina_Stringshare *str = eolian_type_enum_field_c_name_get(efl);
        eina_strbuf_append_char(wbuf, '.');
        eina_strbuf_append(wbuf, str);
        eina_stringshare_del(str);
        eina_stringshare_del(bname);
        return;
     }

   const Eolian_Class *cl = eolian_class_get_by_name(bname);
   const Eolian_Function *fn = NULL;
   Eolian_Function_Type ftype = EOLIAN_UNRESOLVED;
   if (!cl)
     {
        const char *mname;
        if (!strcmp(sfx, ".get")) ftype = EOLIAN_PROP_GET;
        else if (!strcmp(sfx, ".set")) ftype = EOLIAN_PROP_SET;
        if (ftype != EOLIAN_UNRESOLVED)
          {
             eina_stringshare_del(bname);
             mname = sfx - 1;
             while ((mname != refn) && (*mname != '.')) --mname;
             if (mname == refn) goto noref;
             bname = eina_stringshare_add_length(refn, mname - refn);
             cl = eolian_class_get_by_name(bname);
             eina_stringshare_del(bname);
          }
        if (cl)
          {
             char *meth = strndup(mname + 1, sfx - mname - 1);
             fn = eolian_class_function_get_by_name(cl, meth, ftype);
             if (ftype == EOLIAN_UNRESOLVED)
               ftype = eolian_function_type_get(fn);
             free(meth);
          }
     }
   else
     {
        fn = eolian_class_function_get_by_name(cl, sfx + 1, ftype);
        ftype = eolian_function_type_get(fn);
     }

   if (!fn) goto noref;

   Eina_Stringshare *fcn = eolian_function_full_c_name_get(fn, ftype, use_legacy);
   if (!fcn) goto noref;
   eina_strbuf_append(wbuf, fcn);
   eina_stringshare_del(fcn);
   return;
noref:
   eina_strbuf_append(wbuf, refn);
}

static int
_append_section(const char *desc, int ind, int curl, Eina_Strbuf *buf,
                Eina_Strbuf *wbuf, Eina_Bool use_legacy)
{
   while (*desc)
     {
        eina_strbuf_reset(wbuf);
        while (*desc && isspace(*desc) && (*desc != '\n'))
          eina_strbuf_append_char(wbuf, *desc++);
        if (*desc == '\\')
          {
             desc++;
             if ((*desc != '@') && (*desc != '$'))
               eina_strbuf_append_char(wbuf, '\\');
             eina_strbuf_append_char(wbuf, *desc++);
          }
        else if (*desc == '@')
          {
             const char *ref = ++desc;
             if (isalpha(*desc) || (*desc == '_'))
               {
                  eina_strbuf_append(wbuf, "@ref ");
                  while (isalnum(*desc) || (*desc == '.') || (*desc == '_'))
                    ++desc;
                  if (*(desc - 1) == '.') --desc;
                  Eina_Stringshare *refn = eina_stringshare_add_length(ref, desc - ref);
                  _generate_ref(refn, wbuf, use_legacy);
                  eina_stringshare_del(refn);
               }
             else
               eina_strbuf_append_char(wbuf, '@');
          }
        else if (*desc == '$')
          {
             desc++;
             if (isalpha(*desc))
               eina_strbuf_append(wbuf, "@c ");
             else
               eina_strbuf_append_char(wbuf, '$');
          }
        while (*desc && !isspace(*desc))
          eina_strbuf_append_char(wbuf, *desc++);
        int limit = DOC_LIMIT(ind);
        int wlen = eina_strbuf_length_get(wbuf);
        if ((int)(curl + wlen) > limit)
          {
             curl = 3;
             eina_strbuf_append_char(buf, '\n');
             curl += _indent_line(buf, ind);
             eina_strbuf_append(buf, " * ");
             if (*eina_strbuf_string_get(wbuf) == ' ')
               eina_strbuf_remove(wbuf, 0, 1);
          }
        curl += eina_strbuf_length_get(wbuf);
        eina_strbuf_append(buf, eina_strbuf_string_get(wbuf));
        if (*desc == '\n')
          {
             desc++;
             eina_strbuf_append_char(buf, '\n');
             while (*desc == '\n')
               {
                  _indent_line(buf, ind);
                  eina_strbuf_append(buf, " *\n");
                  desc++;
               }
             curl = _indent_line(buf, ind) + 3;
             eina_strbuf_append(buf, " * ");
          }
     }
   return curl;
}

static int
_append_since(const char *since, int indent, int curl, Eina_Strbuf *buf)
{
   if (since)
     {
        eina_strbuf_append_char(buf, '\n');
        _indent_line(buf, indent);
        eina_strbuf_append(buf, " *\n");
        curl = _indent_line(buf, indent);
        eina_strbuf_append(buf, " * @since ");
        eina_strbuf_append(buf, since);
        curl += strlen(since) + sizeof(" * @since ") - 1;
     }
   return curl;
}

static char *
_sanitize_group(const char *group)
{
   if (!group) return NULL;
   char *ret = strdup(group);
   char *p;
   while ((p = strchr(ret, '.'))) *p = '_';
   return ret;
}

static void
_append_group(Eina_Strbuf *buf, char *sgrp, int indent)
{
   if (!sgrp) return;
   eina_strbuf_append(buf, " * @ingroup ");
   eina_strbuf_append(buf, sgrp);
   eina_strbuf_append_char(buf, '\n');
   _indent_line(buf, indent);
   free(sgrp);
}

static void
_gen_doc_brief(const char *summary, const char *since, const char *group,
               int indent, Eina_Strbuf *buf, Eina_Bool use_legacy)
{
   int curl = 4 + indent;
   Eina_Strbuf *wbuf = eina_strbuf_new();
   eina_strbuf_append(buf, "/** ");
   curl = _append_section(summary, indent, curl, buf, wbuf, use_legacy);
   eina_strbuf_free(wbuf);
   curl = _append_since(since, indent, curl, buf);
   char *sgrp = _sanitize_group(group);
   if (((curl + 3) > DOC_LIMIT(indent)) || sgrp)
     {
        eina_strbuf_append_char(buf, '\n');
        _indent_line(buf, indent);
        if (sgrp) eina_strbuf_append(buf, " *");
     }
   if (sgrp)
     {
        eina_strbuf_append_char(buf, '\n');
        _indent_line(buf, indent);
     }
   _append_group(buf, sgrp, indent);
   eina_strbuf_append(buf, " */");
}

static void
_gen_doc_full(const char *summary, const char *description, const char *since,
              const char *group, int indent, Eina_Strbuf *buf, Eina_Bool use_legacy)
{
   int curl = 0;
   Eina_Strbuf *wbuf = eina_strbuf_new();
   eina_strbuf_append(buf, "/**\n");
   curl += _indent_line(buf, indent);
   eina_strbuf_append(buf, " * @brief ");
   curl += sizeof(" * @brief ") - 1;
   _append_section(summary, indent, curl, buf, wbuf, use_legacy);
   eina_strbuf_append_char(buf, '\n');
   _indent_line(buf, indent);
   eina_strbuf_append(buf, " *\n");
   curl = _indent_line(buf, indent);
   eina_strbuf_append(buf, " * ");
   _append_section(description, indent, curl + 3, buf, wbuf, use_legacy);
   curl = _append_since(since, indent, curl, buf);
   eina_strbuf_append_char(buf, '\n');
   _indent_line(buf, indent);
   char *sgrp = _sanitize_group(group);
   if (sgrp)
     {
        eina_strbuf_append(buf, " *\n");
        _indent_line(buf, indent);
     }
   _append_group(buf, sgrp, indent);
   eina_strbuf_append(buf, " */");
   eina_strbuf_free(wbuf);
}

Eina_Strbuf *
docs_generate_full(const Eolian_Documentation *doc, const char *group,
                   int indent, Eina_Bool use_legacy)
{
   if (!doc) return NULL;

   const char *sum = eolian_documentation_summary_get(doc);
   const char *desc = eolian_documentation_description_get(doc);
   const char *since = eolian_documentation_since_get(doc);

   Eina_Strbuf *buf = eina_strbuf_new();
   if (!desc)
     _gen_doc_brief(sum, since, group, indent, buf, use_legacy);
   else
     _gen_doc_full(sum, desc, since, group, indent, buf, use_legacy);
   return buf;
}

Eina_Strbuf *
docs_generate_function(const Eolian_Function *fid, Eolian_Function_Type ftype,
                       int indent, Eina_Bool use_legacy)
{
   const Eolian_Function_Parameter *par = NULL;
   const Eolian_Function_Parameter *vpar = NULL;

   const Eolian_Documentation *doc, *pdoc, *rdoc;

   Eina_Iterator *itr = NULL;
   Eina_Iterator *vitr = NULL;
   Eina_Bool force_out = EINA_FALSE;

   Eina_Strbuf *buf = eina_strbuf_new();
   Eina_Strbuf *wbuf = NULL;

   const char *sum = NULL, *desc = NULL, *since = NULL;

   int curl = 0;

   const char *group = eolian_class_full_name_get(eolian_function_class_get(fid));

   if (ftype == EOLIAN_UNRESOLVED)
     ftype = EOLIAN_METHOD;

   if (ftype == EOLIAN_METHOD)
     {
        doc = eolian_function_documentation_get(fid, EOLIAN_METHOD);
        pdoc = NULL;
     }
   else
     {
        doc = eolian_function_documentation_get(fid, EOLIAN_PROPERTY);
        pdoc = eolian_function_documentation_get(fid, ftype);
        if (!doc && pdoc) doc = pdoc;
        if (pdoc == doc) pdoc = NULL;
     }

   rdoc = eolian_function_return_documentation_get(fid, ftype);

   if (doc)
     {
         sum = eolian_documentation_summary_get(doc);
         desc = eolian_documentation_description_get(doc);
         since = eolian_documentation_since_get(doc);
         if (pdoc && eolian_documentation_since_get(pdoc))
           since = eolian_documentation_since_get(pdoc);
     }

   if (ftype == EOLIAN_METHOD)
     {
        itr = eolian_function_parameters_get(fid);
        if (!itr || !eina_iterator_next(itr, (void**)&par))
          {
             eina_iterator_free(itr);
             itr = NULL;
          }
     }
   else
     {
        itr = eolian_property_keys_get(fid, ftype);
        vitr = eolian_property_values_get(fid, ftype);
        if (!vitr || !eina_iterator_next(vitr, (void**)&vpar))
          {
             eina_iterator_free(vitr);
             vitr = NULL;
         }
     }

   if (!itr || !eina_iterator_next(itr, (void**)&par))
     {
        eina_iterator_free(itr);
        itr = NULL;
     }

   /* when return is not set on getter, value becomes return instead of param */
   if (ftype == EOLIAN_PROP_GET && !eolian_function_return_type_get(fid, ftype))
     {
        if (!eina_iterator_next(vitr, (void**)&vpar))
          {
             /* one value - not out param */
             eina_iterator_free(vitr);
             rdoc = eolian_parameter_documentation_get(vpar);
             vitr = NULL;
             vpar = NULL;
          }
        else
          {
             /* multiple values - always out params */
             eina_iterator_free(vitr);
             vitr = eolian_property_values_get(fid, ftype);
             if (!vitr)
               vpar = NULL;
             else if (!eina_iterator_next(vitr, (void**)&vpar))
               {
                  eina_iterator_free(vitr);
                  vitr = NULL;
                  vpar = NULL;
               }
          }
     }

   if (!par)
     {
        /* no keys, try values */
        itr = vitr;
        par = vpar;
        vitr = NULL;
        vpar = NULL;
        if (ftype == EOLIAN_PROP_GET)
          force_out = EINA_TRUE;
     }

   /* only summary, nothing else; generate standard brief doc */
   if (!desc && !par && !vpar && !rdoc && (ftype == EOLIAN_METHOD || !pdoc))
     {
        _gen_doc_brief(sum ? sum : "No description supplied.", since, group,
                       indent, buf, use_legacy);
        return buf;
     }

   wbuf = eina_strbuf_new();

   eina_strbuf_append(buf, "/**\n");
   curl += _indent_line(buf, indent);
   eina_strbuf_append(buf, " * @brief ");
   curl += sizeof(" * @brief ") - 1;
   _append_section(sum ? sum : "No description supplied.",
                   indent, curl, buf, wbuf, use_legacy);

   eina_strbuf_append_char(buf, '\n');
   if (desc || since || par || rdoc || pdoc)
     {
        _indent_line(buf, indent);
        eina_strbuf_append(buf, " *\n");
     }

   if (desc)
     {
        curl = _indent_line(buf, indent);
        eina_strbuf_append(buf, " * ");
        _append_section(desc, indent, curl + 3, buf, wbuf, use_legacy);
        eina_strbuf_append_char(buf, '\n');
        if (par || rdoc || pdoc || since)
          {
             _indent_line(buf, indent);
             eina_strbuf_append(buf, " *\n");
          }
     }

   if (pdoc)
     {
        const char *pdesc = eolian_documentation_description_get(pdoc);
        curl = _indent_line(buf, indent);
        eina_strbuf_append(buf, " * ");
        _append_section(eolian_documentation_summary_get(pdoc), indent,
            curl + 3, buf, wbuf, use_legacy);
        eina_strbuf_append_char(buf, '\n');
        if (pdesc)
          {
             _indent_line(buf, indent);
             eina_strbuf_append(buf, " *\n");
             curl = _indent_line(buf, indent);
             eina_strbuf_append(buf, " * ");
             _append_section(pdesc, indent, curl + 3, buf, wbuf, use_legacy);
             eina_strbuf_append_char(buf, '\n');
          }
        if (par || rdoc || since)
          {
             _indent_line(buf, indent);
             eina_strbuf_append(buf, " *\n");
          }
     }

   while (par)
     {
        const Eolian_Documentation *adoc = eolian_parameter_documentation_get(par);
        curl = _indent_line(buf, indent);

        Eolian_Parameter_Dir dir = EOLIAN_OUT_PARAM;
        if (!force_out)
          dir = eolian_parameter_direction_get(par);

        switch (dir)
          {
           case EOLIAN_IN_PARAM:
             eina_strbuf_append(buf, " * @param[in] ");
             curl += sizeof(" * @param[in] ") - 1;
             break;
           case EOLIAN_OUT_PARAM:
             eina_strbuf_append(buf, " * @param[out] ");
             curl += sizeof(" * @param[out] ") - 1;
             break;
           case EOLIAN_INOUT_PARAM:
             eina_strbuf_append(buf, " * @param[in,out] ");
             curl += sizeof(" * @param[in,out] ") - 1;
             break;
          }

        const char *nm = eolian_parameter_name_get(par);
        eina_strbuf_append(buf, nm);
        curl += strlen(nm);

        if (adoc)
          {
             eina_strbuf_append_char(buf, ' ');
             curl += 1;
             _append_section(eolian_documentation_summary_get(adoc),
                             indent, curl, buf, wbuf, use_legacy);
          }

        eina_strbuf_append_char(buf, '\n');
        if (!eina_iterator_next(itr, (void**)&par))
          {
             par = NULL;
             if (vpar)
               {
                  eina_iterator_free(itr);
                  itr = vitr;
                  par = vpar;
                  vitr = NULL;
                  vpar = NULL;
                  if (ftype == EOLIAN_PROP_GET)
                    force_out = EINA_TRUE;
               }
          }

        if (!par && (rdoc || since))
          {
             _indent_line(buf, indent);
             eina_strbuf_append(buf, " *\n");
          }
     }
   eina_iterator_free(itr);

   if (rdoc)
     {
        curl = _indent_line(buf, indent);
        eina_strbuf_append(buf, " * @return ");
        curl += sizeof(" * @return ") - 1;
        _append_section(eolian_documentation_summary_get(rdoc), indent, curl,
            buf, wbuf, use_legacy);
        eina_strbuf_append_char(buf, '\n');
        if (since)
          {
             _indent_line(buf, indent);
             eina_strbuf_append(buf, " *\n");
          }
     }

   if (since)
     {
        curl = _indent_line(buf, indent);
        eina_strbuf_append(buf, " * @since ");
        eina_strbuf_append(buf, since);
        eina_strbuf_append_char(buf, '\n');
     }

   _indent_line(buf, indent);
   eina_strbuf_append(buf, " *\n");

   _indent_line(buf, indent);
   _append_group(buf, _sanitize_group(group), indent);
   eina_strbuf_append(buf, " */");
   eina_strbuf_free(wbuf);
   return buf;
}
