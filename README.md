# Demo IoT Senzor
**pro Azure IoT DevKit od MSCHIP AZ3155**

Příklad kódu pro Arduino IoT senzor demonstrující použití Azure IoT Hub a dalších služeb. Vychází z open source Microsoft příkladů a rozšiřuje je. V tomto repozitáři najdete pouze kód pro samotný IoT DevKit. Demonstrace využití takto naprogramovaného zařízení v kombinaci s Azure službami najdete postupně v jiných repo.

## Aktuální funkce
* Odesílání zpráv do IoT Hub ze všech senzorů
* Možnost zastavit a znovu spustit odesílání tlačítkem na zařízení
* Agregace údajů z akcelerometru mimo odesílací cyklus (průměr a absolutní maximum za interval)
* Možnost nastavit odesílací interval tlačítkem na zařízení

## To Do
* Voice Recognition (na tlačítko nahrát hlas a rozeznat mluvčího)
* C2D zprávy s využitím Logic App (například upozornění na událost)
* D2C zprávy s využitím Logic App (například spuštění alarmu)


# Jak začít?
Začněte podle návodu zde: (https://microsoft.github.io/azure-iot-developer-kit/)

Vyzkoušejte si základní příklad GetStarted, který odesílá teplotu a vlhkost.

Následně použijte kód z toho repozitáře pro rozšíření těchto funkcí (například nakopírujte soubory v základním adresáři do vašeho GetStarted a odešlete do zařízení).

# Ovládání a funkce

Po startu se zařízení připojí na nastavenou WiFi síť. Zobrazí IP adresu a informaci o běžícím odesílání. Každá odeslaná zpráva způsobí bliknutí modré diody nalevo od displeje.

(../img/screen0.jpg)

Stisknutím tlačítka B můžete odesílání zastavit a opětovně zpustit.

(../img/screen1.jpg)

Stisknutím tlačítka A se dostanete na druhou obrozovku, v které můžete změnit interval odesílání použitím tlačítka B.

(../img/screen2.jpg)



# Odesílaná datová struktura

DevKit odesílá data ze všech senzorů. Jedná se o odečty uskutečněné v okamžiku odesílacího intervalu. Pro detekci neobvyklého pohybu zařízení pokud toto nastane mimo interval odesílání toto zůstane nezdetekováno. Kód tedy implementuje častější odečty několikrát za vteřinu a pro akcelerometr posílá kromě jedné hodnoty odečtené při odesílacím intervalu také průměr a maximum za všechny odečty v rámci intervalu.

Takto vypadá odesíláný JSON:

```
{
    "deviceId": "AZ3166",
    "messageId": 13,
    "temperature": 22,
    "humidity": 76,
    "preassure": 986,
    "magneto: {
            "x": -2,
            "y": -105,
            "z": 1011
    },
    "acce": {
        "now": {
            "x": -2,
            "y": -105,
            "z": 1011
        },
        "avg": {
            "x": -2,
            "y": -105,
            "z": 1011
        },
        "max": {
            "x": -20,
            "y": -113,
            "z": 1013
        }
    },
    "gyro": {
        "x": -280,
        "y": -1260,
        "z": 1330
    }
}
```