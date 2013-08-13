// inco_tp1.c
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#define MAX 100

//Estructura de atributes es/ha/tiene (ej roja, jugoza, dulce)
struct atributo {
    char atrib [80];
    struct atributo *siguiente;
} at;
//Estructura de objetos (lo que queremos identificar ej manzana)
struct objeto {
    char nombre [80];
    struct atributo *alista; /* Apuntar a la lista de atributos */
} ob;

struct objeto_rechazado {
    char nombre [80];
    char atrib [80]; /* Atributo que causo el rechazo */
    char condicion; /* Era necesario o se descarto por
                     una deduccion previa */
} rj;

struct objeto_rechazado r_base [MAX];
struct objeto base_c [MAX]; /* Base de conocimiento */
int n_pos = -1; /* Posicion en la base de conocimiento */
int r_pos = -1; /* Posicion en la lista de rechazos */
struct atributo *si, *no; /* listas de tiene y no tiene */
struct atributo *siguientesi, *siguienteno;

void introduce();
void pregunta ();
int intenta (struct atributo *p, char *ob);
int sigueno (struct atributo *p);
int libera_lista();
int preg (char *atrib);
char menu();
void salva ();
void carga();
int ini_basec();
int esta_en (char ch, char *s);


int main ()
{
    char ch;
    no = si = 0x00;
    do {
        libera_lista();
        ch = menu();
        switch(ch) {
            case 'i': introduce();
                break;
            case 'p': pregunta();
                break;
            case 's': salva();
                break;
            case 'c': carga();
                break;
        }
    } while (ch != 'x');
    return (0);
}

int libera_lista()
{
    struct atributo *p;
    while (si) {
        p = si -> siguiente;
        free(si);
        si = p;
    }
    while (no) {
        p = no -> siguiente;
        free (no);
        no = p;
    }
    return (0);
}
/*
 * Funcion encargada de crear la base de conocimiento
 */
void introduce()
{
    int t;
    struct atributo *nuevo_attributo, *anterior_p;
    //Iteramos para ingresar varios objetos hasta que ingresamos un objeto vacio
    for (;;) {
        t = obtiene_siguiente();
        if (t == -1) {
            printf ("Fuera de la lista.\n");
            return;
        }
        printf ("Nombre del objeto: ");
        //ingresa el nombre del objeto en la base de conocimientos en el indice t para el nombre
        gets (base_c[t].nombre);
        //si el nombre es vacio termina el loop para ingresar objetos
        if (!*base_c[t].nombre) {
            n_pos--;
            //Salida del for
            break;
        }
        //reserva memoria para el primer attributo del objeto,
        nuevo_attributo = (struct atributo *) malloc(sizeof(at));
        //Checkea disponibilidad de memoria
        if (nuevo_attributo == 0x00) {
            printf ("No hay memoria suficiente.\n");
            return;
        }
        //asigna en el objeto recien ingresado el puntero al primer atributo el cual será ingresado en el proximo paso
        base_c[t].alista = nuevo_attributo;
        printf ("Introduce los atributos del objeto. ENTER para salir\n");
        //Iteramos para ingresar varios atributos hasta ingresar uno vacio
        for (;;) {
            printf (">> ");
            //ingresamos el nombre del attributo
            gets (nuevo_attributo->atrib);
            //si el atributo es vacio salimos del bloque for para insertar atributos para el objeto
            if (!nuevo_attributo->atrib[0]) break;
            //guarda la referencia al atributo en la variable anterior_p
            anterior_p = nuevo_attributo;
            //reserva memoria para el siguiente
            nuevo_attributo->siguiente = (struct atributo *) malloc(sizeof(at));
            //cambia la referencia de p a p->siguiente (la primera vez p queda asignado como primer attributo del objeto)
            nuevo_attributo = nuevo_attributo->siguiente;
            //el siguiente del nuevo attributo es vacio
            nuevo_attributo->siguiente = 0x00;
            if (nuevo_attributo == 0x00) {
                printf ("No hay memoria suficiente.\n");
                return;
            }
        }
        anterior_p->siguiente = 0x00;
    }
    return;
}
/*
 * Funcion encargada de realizar las preguntas al Sistema Experto.
 */
void pregunta ()
{
    int t;
    char ch;
    struct atributo *p;
    for (t=0;t<=n_pos;t++) {
        //busca el primer objeto cargado en el arra base_c
        p = base_c[t].alista;
        
        if (intenta(p, base_c[t].nombre)) {
            printf ("%s concuerda con la actual descripcion\n", base_c[t].nombre);
            printf ("sigo (S/N): ");
            ch = tolower(getchar());
            getchar();
            printf ("\n");
            if (ch == 'n') return;
        }
    }
    printf ("No se ha(n) encontrado (mas) objeto(s)\n");
    return;
}
/*
 * Funcion encargada de comprobar un objeto.
 */
int intenta (struct atributo *p, char *ob)
{
    char respuesta;
    struct atributo *a, *t;
    if (!sigueno(p)) return 0;
    if (!siguesi(p)) return 0;
    while (p) {
        if (preg (p->atrib)) {
            printf ("es/ha/tiene %s? ", p->atrib);
            respuesta = tolower(getchar());
            getchar();
            printf ("\n");
            a = (struct atributo *) malloc(sizeof(at));
            if (!a) {
                printf ("No hay memoria suficiente.\n");
                return (0);
            }
            a->siguiente = 0x00;
            switch(respuesta) {
                case 'n': strcpy (a->atrib, p->atrib);
                    if (!no) {
                        no = a;
                        siguienteno = no;
                    }
                    else {
                        siguienteno->siguiente = a;
                        siguienteno = a;
                    }
                    return (0);
                case 's': strcpy (a->atrib,p->atrib);
                    if (!si) {
                        si = a;
                        siguientesi = si;
                    }
                    else {
                        siguientesi->siguiente = a;
                        siguientesi = a;
                    }
                    p = p->siguiente;
                    break;
                case 'p': razonando (ob);
                    break;
            }
        }
        else p = p->siguiente;
    }
    return 1;
}
/*
 * Funcion que busca un atributo que no tenga el objeto y que este en la lista
 */
int sigueno (struct atributo *p)
{
    struct atributo *a, *t;
    a = no;
    while (a) {
        t = p;
        while (t) {
            if (!strcmp(t->atrib,a->atrib))
                return 0;
            t = t->siguiente;
        }
        a = a->siguiente;
    }
    return 1;
}
/*
 * Funcion que comprueba que tenga los atributos seleccionados
 */
int siguesi (struct atributo *p)
{
    struct atributo *a, *t;
    char ok;
    a = si;
    while (a) {
        ok = 0x00;
        t = p;
        while (t) {
            if (!strcmp(t->atrib,a->atrib))
                ok = 0x01;
            t = t->siguiente;
        }
        if (!ok) return 0;
        a = a->siguiente;
    }
    return 1;
}
/*
 * Funcion que comprueba si el atributo se pregunto con anterioridad
 */
int preg (char *atrib)
{
    struct atributo *p;
    p = si;
    while (p && strcmp(atrib, p->atrib))
        p = p->siguiente;
    if (!p) return 1;
    else return 0;
}
/*
 * Funcion que muestra el motivo por el que se sigue una determinada l'nea de conocimiento.
 */
int razonando (char *ob)
{
    struct atributo *t;
    int i;
    printf ("Intentando %s\n", ob);
    if (si)
        printf ("es/tiene/ha :\n");
    t = si;
    while (t) {
        printf ("%s\n", t->atrib);
        t = t->siguiente;
    }
    if (no)
        printf ("No es/tiene/ha :\n");
    t = no;
    while (t) {
        printf ("%s\n", t->atrib);
        t = t->siguiente;
    }
    for (i=0;i<=r_pos;i++) {
        printf ("%s rechazado porque ", r_base[i].nombre);
        if (r_base[i].condicion == 'n')
            printf ("%s no es un atributo.\n", r_base[i].atrib);
        else
            printf ("%s es un atributo requerido.\n", r_base[i].atrib);
    }
    return (0);
}
/*
 * Funcion que situa el objeto rechazado en la base de datos
 */
int rechaza (char *ob, char *at, char cond)
{
    r_pos++;
    strcpy(r_base[r_pos].nombre, ob);
    strcpy(r_base[r_pos].atrib, at);
    r_base[r_pos].condicion = cond;
    return (0);
}
/*
 * Funcion que obtiene el siguiente indice libre del array de la base de conocimiento
 */
int obtiene_siguiente()
{
    n_pos++;
    if (n_pos < MAX) return n_pos;
    else return -1;
}
/*
 * Funcion con el menu de opciones
 */
char menu()
{
    char ch;
    printf ("(I)ntroduce (P)regunta (S)alva (C)arga e(X)it\n");
    do {
        printf ("Selecciona una opcion: ");
        ch = tolower(getchar());
        getchar();
    } while (!esta_en (ch, "ipscx"));
    printf ("\n");
    return ch;
}
/*
 * Funcion para guardar la base de conocimiento
 */
void salva ()
{
    int t, x;
    struct atributo *p;
    FILE *fp;
    if ((fp = fopen("experto.dat", "w")) == 0) {
        printf ("No puedo crear el archivo\n");
        return;
    }
    printf ("Salvando la base de conocimientos\n");
    for (t=0;t<=n_pos;++t) {
        for (x=0;x<sizeof(base_c[t].nombre);x++)
            if (base_c[t].nombre[x])
                putc (base_c[t].nombre[x], fp);
            else
            {
                putc ('\n', fp);
                break;
            }
        p = base_c[t].alista;
        while (p)
        {
            for (x=0;x<sizeof(p->atrib);x++)
                if (p->atrib[x])
                    putc(p->atrib[x], fp);
                else
                {
                    putc ('\n', fp);
                    break;
                }
            p = p->siguiente;
        }
        putc ('\n', fp);
    }
    putc (0, fp);
    fclose (fp);
    return;
}
/*
 * Funcion que carga una base de conocimiento previamente almacenada
 */
void carga()
{
    int t, x;
    struct atributo *p, *anterior_p;
    FILE *fp;
    if ((fp = fopen("experto.dat", "r")) == 0) {
        printf ("No puedo abrir el archivo.\n");
        return;
    }
    printf ("Cargando la base de conocimientos\n");
    ini_basec();
    for (t=0;t<MAX;++t) {
        if ((base_c[t].nombre[0] = getc(fp)) == 0)
            break;
        for (x=1;x<sizeof(base_c[t].nombre);x++)
            if ((base_c[t].nombre[x] = getc(fp)) == '\n')
            {
                base_c[t].nombre[x] = 0x00;
                break;
            }
        base_c[t].alista = (struct atributo *) malloc(sizeof(at));
        if (!base_c[t].alista)
        {
            printf ("No hay memoria suficiente.\n");
            break;
        }
        p = base_c[t].alista;
        for (;;)
        {
            for (x=0;x<sizeof(p->atrib);x++)
                if ((p->atrib[x] = getc(fp)) == '\n')
                {
                    p->atrib[x] = 0x00;
                    break;
                }
            
            if (!p->atrib[0])
            {
                anterior_p->siguiente=0x00;
                break;
            }
            p->siguiente = (struct atributo *) malloc(sizeof(at));
            if (!p->siguiente)
            {
                printf ("No hay memoria suficiente.\n");
                break;
            }
            anterior_p = p;
            p = p->siguiente;
        }
    }
    fclose (fp);
    n_pos = t - 1;
    return;
}
/*
 * Funcion para inicializar la base de conocimiento
 */
int ini_basec()
{
    int t;
    struct atributo *p, *p2;
    for (t=0;t<=n_pos;t++) {
        p = base_c[t].alista;
        while (p) {
            
            p2 = p;
            free (p);
            p = p2->siguiente;
        }
    }
    return (0);
}
/*
 * Funcion que busca un elemento
 */
int esta_en (char ch, char *s)
{
    while (*s)
        if (ch == *s++)
            return 1;
    return 0;
}