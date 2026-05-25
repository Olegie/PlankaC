#include <string.h>

#include "plankagui.h"

static const unsigned char *pmg_font(char c)
{
    static const unsigned char space[7] = {0, 0, 0, 0, 0, 0, 0};
    static const unsigned char A[7] = {14, 17, 17, 31, 17, 17, 17};
    static const unsigned char B[7] = {30, 17, 17, 30, 17, 17, 30};
    static const unsigned char C[7] = {14, 17, 16, 16, 16, 17, 14};
    static const unsigned char D[7] = {30, 17, 17, 17, 17, 17, 30};
    static const unsigned char E[7] = {31, 16, 16, 30, 16, 16, 31};
    static const unsigned char F[7] = {31, 16, 16, 30, 16, 16, 16};
    static const unsigned char G[7] = {14, 17, 16, 23, 17, 17, 15};
    static const unsigned char H[7] = {17, 17, 17, 31, 17, 17, 17};
    static const unsigned char I[7] = {31, 4, 4, 4, 4, 4, 31};
    static const unsigned char J[7] = {1, 1, 1, 1, 17, 17, 14};
    static const unsigned char K[7] = {17, 18, 20, 24, 20, 18, 17};
    static const unsigned char L[7] = {16, 16, 16, 16, 16, 16, 31};
    static const unsigned char M[7] = {17, 27, 21, 21, 17, 17, 17};
    static const unsigned char N[7] = {17, 25, 21, 19, 17, 17, 17};
    static const unsigned char O[7] = {14, 17, 17, 17, 17, 17, 14};
    static const unsigned char P[7] = {30, 17, 17, 30, 16, 16, 16};
    static const unsigned char Q[7] = {14, 17, 17, 17, 21, 18, 13};
    static const unsigned char R[7] = {30, 17, 17, 30, 20, 18, 17};
    static const unsigned char S[7] = {15, 16, 16, 14, 1, 1, 30};
    static const unsigned char T[7] = {31, 4, 4, 4, 4, 4, 4};
    static const unsigned char U[7] = {17, 17, 17, 17, 17, 17, 14};
    static const unsigned char V[7] = {17, 17, 17, 17, 17, 10, 4};
    static const unsigned char W[7] = {17, 17, 17, 21, 21, 21, 10};
    static const unsigned char X[7] = {17, 17, 10, 4, 10, 17, 17};
    static const unsigned char Y[7] = {17, 17, 10, 4, 4, 4, 4};
    static const unsigned char Z[7] = {31, 1, 2, 4, 8, 16, 31};
    static const unsigned char N0[7] = {14, 17, 19, 21, 25, 17, 14};
    static const unsigned char N1[7] = {4, 12, 4, 4, 4, 4, 14};
    static const unsigned char N2[7] = {14, 17, 1, 2, 4, 8, 31};
    static const unsigned char N3[7] = {30, 1, 1, 14, 1, 1, 30};
    static const unsigned char N4[7] = {2, 6, 10, 18, 31, 2, 2};
    static const unsigned char N5[7] = {31, 16, 30, 1, 1, 17, 14};
    static const unsigned char N6[7] = {6, 8, 16, 30, 17, 17, 14};
    static const unsigned char N7[7] = {31, 1, 2, 4, 8, 8, 8};
    static const unsigned char N8[7] = {14, 17, 17, 14, 17, 17, 14};
    static const unsigned char N9[7] = {14, 17, 17, 15, 1, 2, 12};
    static const unsigned char COLON[7] = {0, 4, 4, 0, 4, 4, 0};
    static const unsigned char DOT[7] = {0, 0, 0, 0, 0, 12, 12};
    static const unsigned char SLASH[7] = {1, 1, 2, 4, 8, 16, 16};
    static const unsigned char DASH[7] = {0, 0, 0, 31, 0, 0, 0};
    static const unsigned char PLUS[7] = {0, 4, 4, 31, 4, 4, 0};
    static const unsigned char STAR[7] = {0, 21, 14, 31, 14, 21, 0};
    static const unsigned char EQ[7] = {0, 0, 31, 0, 31, 0, 0};
    static const unsigned char CARET[7] = {4, 10, 17, 0, 0, 0, 0};
    static const unsigned char UNDER[7] = {0, 0, 0, 0, 0, 0, 31};
    static const unsigned char COMMA[7] = {0, 0, 0, 0, 0, 4, 8};

    if (c >= 'a' && c <= 'z') {
        c = (char)(c - 'a' + 'A');
    }
    switch (c) {
    case 'A': return A;
    case 'B': return B;
    case 'C': return C;
    case 'D': return D;
    case 'E': return E;
    case 'F': return F;
    case 'G': return G;
    case 'H': return H;
    case 'I': return I;
    case 'J': return J;
    case 'K': return K;
    case 'L': return L;
    case 'M': return M;
    case 'N': return N;
    case 'O': return O;
    case 'P': return P;
    case 'Q': return Q;
    case 'R': return R;
    case 'S': return S;
    case 'T': return T;
    case 'U': return U;
    case 'V': return V;
    case 'W': return W;
    case 'X': return X;
    case 'Y': return Y;
    case 'Z': return Z;
    case '0': return N0;
    case '1': return N1;
    case '2': return N2;
    case '3': return N3;
    case '4': return N4;
    case '5': return N5;
    case '6': return N6;
    case '7': return N7;
    case '8': return N8;
    case '9': return N9;
    case ':': return COLON;
    case '.': return DOT;
    case '/': return SLASH;
    case '-': return DASH;
    case '+': return PLUS;
    case '*': return STAR;
    case '=': return EQ;
    case '^': return CARET;
    case '_': return UNDER;
    case ',': return COMMA;
    default: return space;
    }
}

int pmg_text_width(const char *text, int scale)
{
    return (int)strlen(text) * 6 * scale;
}

static void pmg_draw_char(PMG_IMAGE *img, int x, int y, char c, int scale,
    PMG_RGB color)
{
    const unsigned char *rows;
    int row;
    int col;
    int sx;
    int sy;

    rows = pmg_font(c);
    for (row = 0; row < 7; ++row) {
        for (col = 0; col < 5; ++col) {
            if ((rows[row] & (1 << (4 - col))) != 0) {
                for (sy = 0; sy < scale; ++sy) {
                    for (sx = 0; sx < scale; ++sx) {
                        pmg_put_pixel(img, x + col * scale + sx,
                            y + row * scale + sy, color);
                    }
                }
            }
        }
    }
}

void pmg_draw_text(PMG_IMAGE *img, int x, int y, const char *text,
    int scale, PMG_RGB color)
{
    int i;

    for (i = 0; text[i] != '\0'; ++i) {
        pmg_draw_char(img, x + i * 6 * scale, y, text[i], scale, color);
    }
}

void pmg_draw_text_center(PMG_IMAGE *img, PMG_RECT rect,
    const char *text, int scale, PMG_RGB color)
{
    int x;
    int y;

    x = rect.x + (rect.w - pmg_text_width(text, scale)) / 2;
    y = rect.y + (rect.h - 7 * scale) / 2;
    pmg_draw_text(img, x, y, text, scale, color);
}
