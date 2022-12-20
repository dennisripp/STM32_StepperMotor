


double f(double x){                                                  // Definition der Funktion f(x)
    /*
	double wert;
    wert = 10*sin(x)*exp(-x/10);                                     // Eigentliche Definition der Funktion
    return wert;  */
	double wert;																	// Rueckgabewert der Funktion f(x)
}                                                                    // Ende der Funktion f(x)


double f2(double v, double s){
	double wert;
	wert = (pow(v,2))/(2*s);
	return wert;
}


double Int_f(double x){                                              // Definition der analytischen Stammfunktion von f(x)
    double wert;                                                     // (Bem.: benoetigen wir nur zum Vergleich)
    wert = -100*sin(x)*exp(-x/10)/101 - 1000*cos(x)*exp(-x/10)/101;
    return wert;
}

double calc(int cnt_teilintervall, int ug, int og){                                                                                     // Hauptfunktion
    unsigned ts = 3;                                                                            // Anzahl der Teilintervalle
    double a = 2;                                                                               // Untergrenze des bestimmten Integrals
    double b = 4;                                                                               // Obergrenze des bestimmten Integrals
    double h;                                                                                   // Deklaration des h-Wertes
    double x_u;                                                                                 // x-Untergrenze der Teilintegration
    double I_1=0, I_2=0, I_3=0, I_4=0;                                                          // Deklaration der Integrale der vier Approximationsregel

    for(int i = 0; i < ts; ++i){
    	x_u = a + i*h;
    	/*																								// Schleifen Anfang ueber die einzelnen Teilintegrationen
        h=(b-a)/(ts*1);                                                                             // N=1 Trapez-Regel
                                                                                    // Setzen der x-Untergrenze der aktuellen Teilintegration
        I_1 = I_1 + h/2*(f(x_u) + f(x_u+h));                                                        // N=1 Trapez-Regel
        h=(b-a)/(ts*2);                                                                             // N=2 Simpson's-Regel
        I_2 = I_2 + h/3*(f(x_u) + 4*f(x_u+h) + f(x_u+2*h));                                         // N=2 Simpson's-Regel
        h=(b-a)/(ts*3);                                                                             // N=3 Simpson's-3/8-Regel
        I_3 = I_3 + 3*h/8*(f(x_u) + 3*f(x_u+h) + 3*f(x_u+2*h) + f(x_u+3*h));      */                  // N=3 Simpson's-3/8-Regel
        h=(b-a)/(ts*4);                                                                             // N=4 Regel
        I_4 = I_4 + 2*h/45*(7*f(x_u) + 32*f(x_u+h) + 12*f(x_u+2*h) + 32*f(x_u+3*h) + 7*f(x_u+4*h)); // N=4 Regel
    }                                                                                               // Ende for-Schleife
    return I_4;
    /*
    // Ausgaben der berechneten Groessen
    printf("Integral N=1: %14.10f \t Abweichung zum wirklichen Wert: %14.10f \n", I_1, (Int_f(b)-Int_f(a)) - I_1);
    printf("Integral N=2: %14.10f \t Abweichung zum wirklichen Wert: %14.10f \n", I_2, (Int_f(b)-Int_f(a)) - I_2);
    printf("Integral N=3: %14.10f \t Abweichung zum wirklichen Wert: %14.10f \n", I_3, (Int_f(b)-Int_f(a)) - I_3);
    printf("Integral N=4: %14.10f \t Abweichung zum wirklichen Wert: %14.10f \n", I_4, (Int_f(b)-Int_f(a)) - I_4);
    */
}
