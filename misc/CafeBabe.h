/* A packrat parser generated by PackCC 1.3.1 */

#ifndef PCC_INCLUDED_CAFEBABE_H
#define PCC_INCLUDED_CAFEBABE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct pcc_context_tag pcc_context_t;

pcc_context_t *pcc_create(void *auxil);
int pcc_parse(pcc_context_t *ctx, int *ret);
void pcc_destroy(pcc_context_t *ctx);

#ifdef __cplusplus
}
#endif

#endif /* !PCC_INCLUDED_CAFEBABE_H */
