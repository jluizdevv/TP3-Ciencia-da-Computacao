#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

#define CAP 50
#define MAX_P 15
#define CORTE 5

typedef struct {
    float x;
    float y;
} Ponto;

typedef struct No {
    float min_x, max_x, min_y, max_y;
    int qtd;
    Ponto* pts;
    struct No *no, *ne, *so, *se;
} No;

No* novo(float min_x, float max_x, float min_y, float max_y) {
    No* n = (No*)malloc(sizeof(No));
    n->min_x = min_x;
    n->max_x = max_x;
    n->min_y = min_y;
    n->max_y = max_y;
    n->qtd = 0;
    n->pts = NULL;
    n->no = NULL;
    n->ne = NULL;
    n->so = NULL;
    n->se = NULL;
    return n;
}

void montar(No* n, Ponto* pts, int qtd, int prof) {
    if (qtd <= CAP || prof >= MAX_P) {
        n->qtd = qtd;
        if (qtd > 0) {
            n->pts = (Ponto*)malloc(qtd * sizeof(Ponto));
            for (int i = 0; i < qtd; i++) {
                n->pts[i] = pts[i];
            }
        }
        return;
    }

    float mx = (n->min_x + n->max_x) / 2.0f;
    float my = (n->min_y + n->max_y) / 2.0f;

    n->no = novo(n->min_x, mx, my, n->max_y);
    n->ne = novo(mx, n->max_x, my, n->max_y);
    n->so = novo(n->min_x, mx, n->min_y, my);
    n->se = novo(mx, n->max_x, n->min_y, my);

    Ponto* p_no = (Ponto*)malloc(qtd * sizeof(Ponto));
    Ponto* p_ne = (Ponto*)malloc(qtd * sizeof(Ponto));
    Ponto* p_so = (Ponto*)malloc(qtd * sizeof(Ponto));
    Ponto* p_se = (Ponto*)malloc(qtd * sizeof(Ponto));

    int c_no = 0, c_ne = 0, c_so = 0, c_se = 0;

    for (int i = 0; i < qtd; i++) {
        if (pts[i].x <= mx && pts[i].y >= my) {
            p_no[c_no++] = pts[i];
        } else if (pts[i].x > mx && pts[i].y >= my) {
            p_ne[c_ne++] = pts[i];
        } else if (pts[i].x <= mx && pts[i].y < my) {
            p_so[c_so++] = pts[i];
        } else if (pts[i].x > mx && pts[i].y < my) {
            p_se[c_se++] = pts[i];
        }
    }

    int criar = (prof < CORTE);

    #pragma omp task shared(n) if(criar)
    montar(n->no, p_no, c_no, prof + 1);

    #pragma omp task shared(n) if(criar)
    montar(n->ne, p_ne, c_ne, prof + 1);

    #pragma omp task shared(n) if(criar)
    montar(n->so, p_so, c_so, prof + 1);

    #pragma omp task shared(n) if(criar)
    montar(n->se, p_se, c_se, prof + 1);

    #pragma omp taskwait

    free(p_no);
    free(p_ne);
    free(p_so);
    free(p_se);
}

int toca(No* n, Ponto p, float raio) {
    float tx = p.x;
    float ty = p.y;

    if (p.x < n->min_x) tx = n->min_x;
    else if (p.x > n->max_x) tx = n->max_x;

    if (p.y < n->min_y) ty = n->min_y;
    else if (p.y > n->max_y) ty = n->max_y;

    float dx = p.x - tx;
    float dy = p.y - ty;
    float dist = sqrt(dx * dx + dy * dy);

    return dist <= raio;
}

void buscar(No* n, Ponto p, float raio, int* achou) {
    if (!toca(n, p, raio)) {
        return;
    }

    if (n->no == NULL) {
        for (int i = 0; i < n->qtd; i++) {
            float dx = n->pts[i].x - p.x;
            float dy = n->pts[i].y - p.y;
            if (sqrt(dx * dx + dy * dy) <= raio) {
                (*achou)++;
            }
        }
        return;
    }

    buscar(n->no, p, raio, achou);
    buscar(n->ne, p, raio, achou);
    buscar(n->so, p, raio, achou);
    buscar(n->se, p, raio, achou);
}

void limpar(No* n) {
    if (n == NULL) return;
    if (n->pts != NULL) free(n->pts);
    limpar(n->no);
    limpar(n->ne);
    limpar(n->so);
    limpar(n->se);
    free(n);
}

int main() {
    int total_pts = 100000;
    Ponto* pts = (Ponto*)malloc(total_pts * sizeof(Ponto));

    for (int i = 0; i < total_pts; i++) {
        pts[i].x = ((float)rand() / RAND_MAX) * 1000.0f;
        pts[i].y = ((float)rand() / RAND_MAX) * 1000.0f;
    }

    No* raiz = novo(0.0f, 1000.0f, 0.0f, 1000.0f);

    double t1 = omp_get_wtime();

    #pragma omp parallel
    {
        #pragma omp single
        montar(raiz, pts, total_pts, 0);
    }

    double t2 = omp_get_wtime();

    float raio = 10.0f;
    int vizinhos = 0;

    double t3 = omp_get_wtime();

    #pragma omp parallel for reduction(+:vizinhos)
    for (int i = 0; i < total_pts; i++) {
        int local = 0;
        buscar(raiz, pts[i], raio, &local);
        vizinhos += local;
    }

    double t4 = omp_get_wtime();

    printf("Construcao: %f seg\n", t2 - t1);
    printf("Consulta: %f seg\n", t4 - t3);
    printf("Vizinhos (raio %.1f): %d\n", raio, vizinhos);

    limpar(raiz);
    free(pts);

    return 0;
}