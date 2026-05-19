program ProyectoLP {
    int contador = 0;
    int limite = 3;
    float promedio = 0.0;
    bool activo = true;
    string nombre = "Angel";

    if (activo == true) {
        while (contador < limite) {
            if (contador < 2) {
                promedio = promedio + 1.5;
            } else if (contador == 2) {
                promedio = promedio + 2.0;
            } else {
                promedio = promedio + 0.5;
            }

            contador = contador + 1;
        }
    } else {
        contador = 0;
    }

    for (contador = 0; contador < limite; contador = contador + 1) {
        do {
            promedio = promedio + 1.0;
        } while (promedio < 10.0);
    }
}