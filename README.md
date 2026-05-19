# Proyecto de Compiladores: Front-End de Compilador

**Alumno:** López Palacios Angel Gabriel  
**Extensión personalizada:** `.lp`  
**Lenguaje de implementación:** C  
**Tipo de lenguaje diseñado:** Fuertemente tipado  

## 1. Descripción general

Este proyecto implementa la fase Front-End de un compilador para un lenguaje de programación fuertemente tipado.

El compilador realiza:

1. Análisis léxico.
2. Análisis sintáctico.
3. Construcción de Árbol de Sintaxis Abstracta, AST.
4. Análisis semántico.
5. Validación de tipos.
6. Control de existencia de variables.
7. Manejo de ámbitos.
8. Validación de anidamiento máximo de tres niveles en estructuras de control.

El lenguaje acepta archivos con extensión `.lp`, formada a partir de las iniciales de los apellidos:

- López
- Palacios

Por eso, los programas fuente deben guardarse con extensión:

```text
.lp