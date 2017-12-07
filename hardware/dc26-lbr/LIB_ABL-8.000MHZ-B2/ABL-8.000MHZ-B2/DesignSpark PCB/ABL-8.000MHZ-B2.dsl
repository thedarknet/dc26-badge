SamacSys ECAD Model
649388/28562/2.18/2/4/Crystal or Oscillator

DESIGNSPARK_INTERMEDIATE_ASCII

(asciiHeader
	(fileUnits MM)
)
(library Library_1
	(padStyleDef "c110_h66"
		(holeDiam 0.66)
		(padShape (layerNumRef 1) (padShapeType Ellipse)  (shapeWidth 1.1) (shapeHeight 1.1))
		(padShape (layerNumRef 16) (padShapeType Ellipse)  (shapeWidth 1.1) (shapeHeight 1.1))
	)
	(textStyleDef "Default"
		(font
			(fontType Stroke)
			(fontFace "Helvetica")
			(fontHeight 50 mils)
			(strokeWidth 5 mils)
		)
	)
	(patternDef "HC/49US_(AT49)PTH" (originalName "HC/49US_(AT49)PTH")
		(multiLayer
			(pad (padNum 1) (padStyleRef c110_h66) (pt -2.44, 0) (rotation 90))
			(pad (padNum 2) (padStyleRef c110_h66) (pt 2.44, 0) (rotation 90))
		)
		(layerContents (layerNumRef 18)
			(attr "RefDes" "RefDes" (pt -1.048, 0.358) (textStyleRef "Default") (isVisible True))
		)
		(layerContents (layerNumRef 28)
			(line (pt -3.75 2.5) (pt 3.75 2.5) (width 0.254))
		)
		(layerContents (layerNumRef 28)
			(line (pt -3.75 -2.5) (pt 3.75 -2.5) (width 0.254))
		)
		(layerContents (layerNumRef 28)
			(arc (pt -3.35112, 0) (radius 2.53162) (startAngle 99.1) (sweepAngle 161.9) (width 0.254))
		)
		(layerContents (layerNumRef 28)
			(arc (pt 3.35112, 0) (radius 2.53162) (startAngle 80.9) (sweepAngle -161.9) (width 0.254))
		)
		(layerContents (layerNumRef 18)
			(line (pt -3.75 2.5) (pt 3.75 2.5) (width 0.254))
		)
		(layerContents (layerNumRef 18)
			(line (pt -3.75 -2.5) (pt 3.75 -2.5) (width 0.254))
		)
		(layerContents (layerNumRef 18)
			(arc (pt 3.35142, 0) (radius 2.53157) (startAngle 279.1) (sweepAngle 161.9) (width 0.254))
		)
		(layerContents (layerNumRef 18)
			(arc (pt -3.35143, 0) (radius 2.53157) (startAngle 99.1) (sweepAngle 161.9) (width 0.254))
		)
	)
	(symbolDef "ABL-8.000MHZ-B2" (originalName "ABL-8.000MHZ-B2")

		(pin (pinNum 1) (pt 0 mils 0 mils) (rotation 0) (pinLength 200 mils) (pinDisplay (dispPinName true)) (pinDes (text (pt 175 mils 0 mils) (rotation 0) (justify "Right") (textStyleRef "Default"))) (pinName (text (pt 225 mils -25 mils) (rotation 0) (justify "Left") (textStyleRef "Default"))
		))
		(pin (pinNum 2) (pt 700 mils 0 mils) (rotation 180) (pinLength 200 mils) (pinDisplay (dispPinName true)) (pinDes (text (pt 525 mils 0 mils) (rotation 0) (justify "Left") (textStyleRef "Default"))) (pinName (text (pt 500 mils -25 mils) (rotation 0) (justify "Right") (textStyleRef "Default"))
		))

		(line (pt 200 mils 100 mils) (pt 500 mils 100 mils) (width 10 mils))
		(line (pt 500 mils 100 mils) (pt 500 mils -100 mils) (width 10 mils))
		(line (pt 500 mils -100 mils) (pt 200 mils -100 mils) (width 10 mils))
		(line (pt 200 mils -100 mils) (pt 200 mils 100 mils) (width 10 mils))

		(attr "RefDes" "RefDes" (pt 550 mils 350 mils) (isVisible True) (textStyleRef "Default"))

	)

	(compDef "ABL-8.000MHZ-B2" (originalName "ABL-8.000MHZ-B2") (compHeader (numPins 2) (numParts 1) (refDesPrefix Y)
		)
		(compPin "1" (pinName "1") (partNum 1) (symPinNum 1) (gateEq 0) (pinEq 0) (pinType Bidirectional))
		(compPin "2" (pinName "2") (partNum 1) (symPinNum 2) (gateEq 0) (pinEq 0) (pinType Bidirectional))
		(attachedSymbol (partNum 1) (altType Normal) (symbolName "ABL-8.000MHZ-B2"))
		(attachedPattern (patternNum 1) (patternName "HC/49US_(AT49)PTH")
			(numPads 2)
			(padPinMap
				(padNum 1) (compPinRef "1")
				(padNum 2) (compPinRef "2")
			)
		)
		(attr "Supplier_Name" "RS")
		(attr "RS Part Number" "")
		(attr "Manufacturer_Name" "ABRACON")
		(attr "Manufacturer_Part_Number" "ABL-8.000MHZ-B2")
		(attr "Allied_Number" "")
		(attr "Other Part Number" "")
		(attr "Description" "ABRACON - ABL-8.000MHZ-B2 - CRYSTAL, 8MHZ, 18PF, HC-49US")
		(attr "Datasheet Link" "http://www.abracon.com/Resonators/ABL.pdf")
		(attr "3D Package" "")
	)

)
