//Inicia la pantalla de la placa de expansión NaturalScience
NaturalScience.initDisplay()
//Si se quire mostrar texto en la pantalla de la placa de expansión NaturalScience
// NaturalScience.showUserText(1, "DFRobot")
basic.forever(function () {
    //Si se quire mostrar alguna medicion, ejemplo la presión en la pantalla de la placa de expansión NaturalScience
    // NaturalScience.showUserText(2, "Pressure: " + NaturalScience.readBME280Data(NaturalScience.BME280Data.Pressure))
    serial.writeString(NaturalScience.readBME280Data(NaturalScience.BME280Data.Pressure) +
        "," + NaturalScience.readBME280Data(NaturalScience.BME280Data.Temperature) +
        "," + NaturalScience.readBME280Data(NaturalScience.BME280Data.Humidity) +
        "," + NaturalScience.TemperatureNumber() + //Digital temperature
        "," + NaturalScience.getNoise() + //Sound intensity
        "," + NaturalScience.getUV() + //UV intensity
        "," + NaturalScience.getC()) //light Intensity
    serial.writeLine("")
    basic.pause(300)
})
