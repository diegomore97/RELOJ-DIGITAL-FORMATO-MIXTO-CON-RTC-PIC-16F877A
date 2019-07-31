
void escribe_rtc(unsigned char direccion, unsigned char dato) {
    i2c_inicia_com();
    i2c_envia_dato(0xD0); // Codigo del RTC
    i2c_envia_dato(direccion);
    i2c_envia_dato(dato);
    i2c_detener();
}

/*
 * Se encarga de leer una posicion del RTC
 */
unsigned char leer_rtc(unsigned char direccion) {
    unsigned char dato;
    i2c_inicia_com();
    i2c_envia_dato(0xD0); // Codigo del RTC
    i2c_envia_dato(direccion);
    i2c_reinicia_com();
    i2c_envia_dato(0xD1);
    dato = i2c_recibe_dato();
    i2c_detener();
    return dato;
}

/*
 * Toma un dato del RTC y lo devuleve en decimal
 */
unsigned char convertirDato(unsigned char dato) {
    unsigned char datoR = 0;
    datoR = (dato & 0xF0) >> 4;
    datoR = (datoR * 10) + (dato & 0x0F);
    return datoR;
}
