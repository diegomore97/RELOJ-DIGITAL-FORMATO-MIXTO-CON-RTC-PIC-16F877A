#include "config.h"
#include <xc.h>
#define _XTAL_FREQ 4000000
#include "I2C.h"
#include "RTC.h"

#define RETARDO 3 //CAMBIO DE DISPLAY CADA 5 MS || 45 segundos en Proteus
#define RETARDO_PARPADEAR 5

#define BOTON_SET PORTBbits.RB0 //BOTON SET

#define BOTON_INCREMENTAR PORTBbits.RB1 //BOTON INCREMENTAR
#define BOTON_TEMPERATURA PORTBbits.RB1 //BOTON TEMPERATURA

#define BOTON_DECREMENTAR PORTBbits.RB2 //BOTON DECREMENTAR
#define BOTON_FORMATO PORTBbits.RB2 //BOTON FORMATO HORA

//son 5 segundos en tiempo real y 50 para simulacion en Proteus

unsigned char numeros[] = {63, 6, 91, 79, 102, 109, 125, 71, 127, 103};
// 0  1   2   3   4    5    6    7   8    9

signed char* contDecHoraMostrar, *contHoraMostrar, *digitoActual;
signed char contDecHora = 1, contHora = 2, contDecMin = 0, contMin = 0;
unsigned char contadorBotonSet, formato, AmPm;
signed char contDecHoraAux, contHoraAux;


void dameTemperatura(void);
void controlBotones(void);
void mostrarDigitos(void);
void validaMinutos(void);
void validaDecenasMinutos(void);
void validaHoras(void);
void parpadearDigitos(void);
void convertirFormato(void);
void verificaAmPm(void);
void dameHoraActual(void); //MODULO RTC 3231
void setRtcDefault(void);
void setRtcHora(void);
void setRtcMinutos(void);

void validaMinutos(void) {
    if (*digitoActual == -1) {
        *digitoActual = 9;
    } else if (*digitoActual == 10) {
        *digitoActual = 0;
    }
}

void validaDecenasMinutos(void) {
    if (*digitoActual == -1) {
        *digitoActual = 5;
    } else if (*digitoActual == 6) {
        *digitoActual = 0;
    }
}

void validaHoras(void) {
    if (contDecHora == 1 && *digitoActual == -1) {
        contDecHora = 0;
        *digitoActual = 9;
    } else if (contDecHora == 2 && *digitoActual == 4) {
        contDecHora = 0;
        *digitoActual = 0;
    } else if (contDecHora == 2 && *digitoActual == -1) {
        contDecHora = 1;
        *digitoActual = 9;
    } else if (!contDecHora && *digitoActual == -1) {
        contDecHora = 2;
        *digitoActual = 3;
    } else if (contDecHora == 1 && *digitoActual == 10) {
        contDecHora = 2;
        *digitoActual = 0;
    } else if (!contDecHora && *digitoActual == 10) {
        contDecHora = 1;
        *digitoActual = 0;
    }
}

void dameTemperatura(void) {

    while (BOTON_TEMPERATURA); //ANTIREBOTE

    short int repeticiones = 400;
    unsigned char unidades, decenas, temperatura;

    temperatura = leer_rtc(0x11);

    unidades = temperatura % 10;
    decenas = (temperatura / 10) % 10;

    while (repeticiones) {

        PORTD = numeros[unidades];
        PORTA = 1;
        __delay_ms(RETARDO);

        if (decenas) {
            PORTD = numeros[decenas];
            PORTA = 2;
            __delay_ms(RETARDO);
        }

        repeticiones--;

    }

}

void controlBotones(void) {

    if (BOTON_SET) //BOTON SET
    {

        while (BOTON_SET); //ANTIREBOTE

        contadorBotonSet++;

        switch (contadorBotonSet) {

            case 1:
                digitoActual = &contMin;
                break;

            case 2:
                digitoActual = &contDecMin;
                break;

            case 3:
                digitoActual = &contHora;
                break;

            default:
                contadorBotonSet = 0;
                setRtcHora();
                setRtcMinutos();
                break;

        }
    } else if (BOTON_INCREMENTAR) //INCREMENTAR DIGITO
    {

        while (BOTON_INCREMENTAR); //ANTIREBOTE

        (*digitoActual)++;

        switch (contadorBotonSet) {
            case 1:
                validaMinutos();
                break;

            case 2:
                validaDecenasMinutos();
                break;

            case 3:
                validaHoras();
                break;
        }



    } else if (BOTON_DECREMENTAR) //DECREMENTAR DIGITO
    {
        while (BOTON_DECREMENTAR); //ANTIREBOTE

        (*digitoActual)--;

        switch (contadorBotonSet) {
            case 1:
                validaMinutos();
                break;

            case 2:
                validaDecenasMinutos();
                break;

            case 3:
                validaHoras();
                break;
        }
    }


}

void parpadearDigitos(void) {

    unsigned char repeticiones = 5;
    while (repeticiones) {

        PORTD = numeros[contMin];

        if (contadorBotonSet == 1) {
            if (repeticiones == 1) {
                PORTA = 1;
                __delay_ms(RETARDO_PARPADEAR);
            }
        } else {
            PORTA = 1;
            __delay_ms(RETARDO_PARPADEAR);
        }

        PORTD = numeros[contDecMin];

        if (contadorBotonSet == 2) {
            if (repeticiones == 1) {
                PORTA = 2;
                __delay_ms(RETARDO_PARPADEAR);
            }
        } else {
            PORTA = 2;
            __delay_ms(RETARDO_PARPADEAR);
        }

        PORTD = numeros[(*contHoraMostrar)] + 128;

        if (contadorBotonSet == 3) {
            if (repeticiones == 1) {
                PORTA = 4;
                __delay_ms(RETARDO_PARPADEAR);
            }
        } else {
            PORTA = 4;
            __delay_ms(RETARDO_PARPADEAR);
        }

        PORTD = numeros[(*contDecHoraMostrar)];

        if (contadorBotonSet == 3) {
            if (repeticiones == 1) {
                PORTA = 8;
                __delay_ms(RETARDO_PARPADEAR);
            }
        } else {
            PORTA = 8;
            __delay_ms(RETARDO_PARPADEAR);
        }

        repeticiones--;
    }
}

void mostrarDigitos(void) {
    //MULTIPLEXACION

    PORTD = numeros[contMin];
    PORTA = 1;
    __delay_ms(RETARDO);

    PORTD = numeros[contDecMin];
    PORTA = 2;
    __delay_ms(RETARDO);

    PORTD = numeros[(*contHoraMostrar)] + 128;
    PORTA = 4;
    __delay_ms(RETARDO);

    PORTD = numeros[(*contDecHoraMostrar)];
    PORTA = 8;
    __delay_ms(RETARDO);

}

void verificaAmPm(void) {

    if (((contHora >= 0 && contHora <= 9)&&!contDecHora) || ((contHora >= 0 && contHora <= 1) && contDecHora == 1)) //AM
    {
        AmPm = 0;
        PORTC = 1;
    } else //PM
    {
        AmPm = 1;
        PORTC = 2;
    }
}

void convertirFormato(void) {
    unsigned char numeroEvaluar = (contDecHora * 10) + contHora;
    switch (numeroEvaluar) {
        case 0:
            contDecHoraAux = 1;
            contHoraAux = 2;
            break;

        case 1:
            contDecHoraAux = 0;
            contHoraAux = 1;
            break;

        case 2:
            contDecHoraAux = 0;
            contHoraAux = 2;
            break;

        case 3:
            contDecHoraAux = 0;
            contHoraAux = 3;
            break;

        case 4:
            contDecHoraAux = 0;
            contHoraAux = 4;
            break;

        case 5:
            contDecHoraAux = 0;
            contHoraAux = 5;
            break;

        case 6:
            contDecHoraAux = 0;
            contHoraAux = 6;
            break;

        case 7:
            contDecHoraAux = 0;
            contHoraAux = 7;
            break;

        case 8:
            contDecHoraAux = 0;
            contHoraAux = 8;
            break;

        case 9:
            contDecHoraAux = 0;
            contHoraAux = 9;
            break;

        case 10:
            contDecHoraAux = 1;
            contHoraAux = 0;
            break;

        case 11:
            contDecHoraAux = 1;
            contHoraAux = 1;
            break;

        case 12:
            contDecHoraAux = 1;
            contHoraAux = 2;
            break;

        case 13:
            contDecHoraAux = 0;
            contHoraAux = 1;
            break;

        case 14:
            contDecHoraAux = 0;
            contHoraAux = 2;
            break;

        case 15:
            contDecHoraAux = 0;
            contHoraAux = 3;
            break;

        case 16:
            contDecHoraAux = 0;
            contHoraAux = 4;
            break;

        case 17:
            contDecHoraAux = 0;
            contHoraAux = 5;
            break;

        case 18:
            contDecHoraAux = 0;
            contHoraAux = 6;
            break;

        case 19:
            contDecHoraAux = 0;
            contHoraAux = 7;
            break;

        case 20:
            contDecHoraAux = 0;
            contHoraAux = 8;
            break;

        case 21:
            contDecHoraAux = 0;
            contHoraAux = 9;
            break;

        case 22:
            contDecHoraAux = 1;
            contHoraAux = 0;
            break;

        case 23:
            contDecHoraAux = 1;
            contHoraAux = 1;
            break;

    }

    contDecHoraMostrar = &contDecHoraAux;
    contHoraMostrar = &contHoraAux;
}

void dameHoraActual(void) //RTC DS3231
{
    unsigned char auxMin, auxHora;

    auxMin = convertirDato(leer_rtc(0x01));
    contMin = auxMin % 10;
    contDecMin = (auxMin / 10) % 10;

    auxHora = convertirDato(leer_rtc(0x02));
    contHora = auxHora % 10;
    contDecHora = (auxHora / 10) % 10;
}

void setRtcDefault(void) {
    unsigned char horaRtc;

    horaRtc = ((1) & 0x0F) << 4;

    horaRtc |= (2) & 0x0F;

    escribe_rtc(0x02, horaRtc); //HORA: 12

    escribe_rtc(0x01, 0); //MINUTOS: 0 MINUTOS 
    escribe_rtc(0x00, 0); //SEGUNDOS: 0 SEGUNDOS 
}

void setRtcHora(void) {

    unsigned char horaRtc;

    horaRtc = ((contDecHora) & 0x0F) << 4;
    horaRtc |= (contHora) & 0x0F;
    escribe_rtc(0x02, horaRtc);

}

void setRtcMinutos(void) {
    unsigned char minutosRtc;

    minutosRtc = ((contDecMin) & 0x0F) << 4;
    minutosRtc |= (contMin) & 0x0F;
    escribe_rtc(0x01, minutosRtc);

}

void main(void) {

    TRISB = 1; //PUERTO B COMO ENTRADA  | LECTURA BOTONES
    TRISD = 0; //PUERTO D COMO SALIDA   | CONTROL DISPLAYS
    TRISA = 0; //PUERTO A COMO ENTRADA  | CONTROL TRANSISTORES
    ADCON1bits.PCFG = 0b0111; //TODO DIGITAL
    TRISCbits.TRISC0 = 0; //INDICADOR AM
    TRISCbits.TRISC1 = 0; //INDICADOR PM

    i2c_iniciar();

    contDecHoraMostrar = &contDecHora;
    contHoraMostrar = &contHora;

    PORTA = 0;
    PORTB = 0;
    PORTC = 0;
    PORTD = 0;

    //setRtcDefault(); //Programar el pic sin comentar esta linea y despues volver a 
    //Programar el pic con esta linea comentada

    while (1) {

        __delay_ms(3);

        if (BOTON_TEMPERATURA&&!contadorBotonSet) //BOTON MOSTRAR TEMPERATURA
        {
            dameTemperatura();
        } else if (BOTON_FORMATO&&!contadorBotonSet) //BOTON PARA CAMBIO DE FORMATO
        {
            while (BOTON_FORMATO); //ANTIREBOTE
            formato = ~formato;
        } else if ((BOTON_SET) || ((BOTON_INCREMENTAR || BOTON_DECREMENTAR) && contadorBotonSet))
            controlBotones(); //BOTONES PARA CAMBIAR LA HORA

        if (!formato) {
            contDecHoraMostrar = &contDecHora;
            contHoraMostrar = &contHora;
            PORTC = 0; //APAGAR INDICADOR DE AM|PM CUANDO EL FORMATO SEA DE 24 HRS.
        } else {
            verificaAmPm();
            convertirFormato();
        }

        if (!contadorBotonSet) {
            dameHoraActual();
            mostrarDigitos();
        } else {
            parpadearDigitos();
        }
    }
    return;
}