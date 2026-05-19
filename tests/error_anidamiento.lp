program ErrorAnidamiento {
    int contador = 0;
    int limite = 5;
    bool activo = true;

    if (activo == true) {
        while (contador < limite) {
            if (contador < 3) {
                for (contador = 0; contador < limite; contador = contador + 1) {
                    contador = contador + 1;
                }
            }
        }
    }
}