/*
  ==============================================================================

  This is an automatically generated file created by the Jucer!

  Creation date:  16 Jul 2009 1:59:01 am

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Jucer version: 1.11

  ------------------------------------------------------------------------------

  The Jucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright 2004-6 by Raw Material Software ltd.

  ==============================================================================
*/

//[Headers] You can add your own extra header files here...
//[/Headers]

#include "MainComponent.h"
#include "scopeComponent.h"
#include "directionscopeComponent.h"
#include "powerscopeComponent.h"
#include "radarComponent.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
MainComponent::MainComponent ()
    : groupComponent5 (0),
      groupComponent3 (0),
      groupComponent2 (0),
      groupComponent (0),
      signalScope (0),
      connectButton (0),
      textEditor (0),
      directionscope (0),
      groupComponent4 (0),
      powerScope (0),
      quitButton (0),
      radarScreen (0)
{
    addAndMakeVisible (groupComponent5 = new GroupComponent (T("new group"),
                                                             T("Global")));

    addAndMakeVisible (groupComponent3 = new GroupComponent (T("new group"),
                                                             T("Direction")));

    addAndMakeVisible (groupComponent2 = new GroupComponent (T("new group"),
                                                             T("Signal (dB)")));

    addAndMakeVisible (groupComponent = new GroupComponent (String::empty,
                                                            T("Host")));

    addAndMakeVisible (signalScope = new Viewport (String::empty));
    signalScope->setScrollBarsShown (false, false);
    signalScope->setViewedComponent (new scopeComponent());

    addAndMakeVisible (connectButton = new TextButton (String::empty));
    connectButton->setButtonText (T("Connect"));
    connectButton->addButtonListener (this);
    connectButton->setColour (TextButton::buttonColourId, Colours::grey);

    addAndMakeVisible (textEditor = new TextEditor (T("new text editor")));
    textEditor->setMultiLine (false);
    textEditor->setReturnKeyStartsNewLine (false);
    textEditor->setReadOnly (false);
    textEditor->setScrollbarsShown (true);
    textEditor->setCaretVisible (true);
    textEditor->setPopupMenuEnabled (true);
    textEditor->setText (T("linuxlightning.homedns.org"));

    addAndMakeVisible (directionscope = new Viewport (String::empty));
    directionscope->setScrollBarsShown (false, false);
    directionscope->setViewedComponent (new directionscopeComponent());

    addAndMakeVisible (groupComponent4 = new GroupComponent (String::empty,
                                                             T("Power (dB)")));

    addAndMakeVisible (powerScope = new Viewport (String::empty));
    powerScope->setScrollBarsShown (false, false);
    powerScope->setViewedComponent (new powerscopeComponent());

    addAndMakeVisible (quitButton = new TextButton (String::empty));
    quitButton->setButtonText (T("Quit"));
    quitButton->addButtonListener (this);
    quitButton->setColour (TextButton::buttonColourId, Colours::grey);

    addAndMakeVisible (radarScreen = new Viewport (String::empty));
    radarScreen->setScrollBarsShown (false, false);
    radarScreen->setViewedComponent (new radarComponent());


    //[UserPreSize]
    //[/UserPreSize]

    setSize (750, 710);

    //[Constructor] You can add your own custom stuff here..
	Dataconnection.SetScope((scopeComponent*)signalScope->getViewedComponent());
	Dataconnection.SetPowerScope((powerscopeComponent*)powerScope->getViewedComponent());
	Dataconnection.SetDirectionScope((directionscopeComponent*)directionscope->getViewedComponent());
	Dataconnection.SetRadarScope((radarComponent*)radarScreen->getViewedComponent());
    //[/Constructor]
}

MainComponent::~MainComponent()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
	connection.Close();
    //[/Destructor_pre]

    deleteAndZero (groupComponent5);
    deleteAndZero (groupComponent3);
    deleteAndZero (groupComponent2);
    deleteAndZero (groupComponent);
    deleteAndZero (signalScope);
    deleteAndZero (connectButton);
    deleteAndZero (textEditor);
    deleteAndZero (directionscope);
    deleteAndZero (groupComponent4);
    deleteAndZero (powerScope);
    deleteAndZero (quitButton);
    deleteAndZero (radarScreen);

    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void MainComponent::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colours::white);

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void MainComponent::resized()
{
    groupComponent5->setBounds (8, 312, 736, 392);
    groupComponent3->setBounds (512, 8, 232, 240);
    groupComponent2->setBounds (8, 8, 232, 240);
    groupComponent->setBounds (8, 248, 484, 62);
    signalScope->setBounds (24, 32, 200, 200);
    connectButton->setBounds (352, 264, 120, 32);
    textEditor->setBounds (24, 264, 320, 32);
    directionscope->setBounds (528, 32, 200, 200);
    groupComponent4->setBounds (260, 8, 232, 240);
    powerScope->setBounds (276, 32, 200, 200);
    quitButton->setBounds (624, 264, 120, 32);
    radarScreen->setBounds (16, 328, 720, 360);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void MainComponent::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == connectButton)
    {
        //[UserButtonCode_connectButton] -- add your button handler code here..
		const char *s=textEditor->getText();
		if (connection.GetSd()==-1) {
			if (connection.Open((char *)s,4713)) {
				Dataconnection.Start(&connection);
				startTimer(100);
				connectButton->setButtonText(T("Disconnect"));
			}
		} else {
			stopTimer();
			Dataconnection.Stop();
			connection.Close();
			((radarComponent*)radarScreen->getViewedComponent())->ClearStations();
			radarScreen->repaint();
			connectButton->setButtonText(T("Connect"));
		}
        //[/UserButtonCode_connectButton]
    }
    else if (buttonThatWasClicked == quitButton)
    {
        //[UserButtonCode_quitButton] -- add your button handler code here..

        JUCEApplication::quit();

        //[/UserButtonCode_quitButton]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
void MainComponent::timerCallback()
{
	if (connection.GetSd()==-1) {
		stopTimer();
		((radarComponent*)radarScreen->getViewedComponent())->ClearStations();
		connectButton->setButtonText(T("Connect"));
	}

	if (((scopeComponent*)(signalScope->getViewedComponent()))->DataChanged())
		signalScope->repaint();
	if (((powerscopeComponent*)(powerScope->getViewedComponent()))->DataChanged())
		powerScope->repaint();
	if (((directionscopeComponent*)(directionscope->getViewedComponent()))->DataChanged())
		directionscope->repaint();
	radarScreen->repaint();
}
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Jucer information section --

    This is where the Jucer puts all of its metadata, so don't change anything in here!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="MainComponent" componentName=""
                 parentClasses="public Component, public Timer" constructorParams=""
                 variableInitialisers="" snapPixels="8" snapActive="1" snapShown="1"
                 overlayOpacity="0.330000013" fixedSize="1" initialWidth="750"
                 initialHeight="710">
  <BACKGROUND backgroundColour="ffffffff"/>
  <GROUPCOMPONENT name="new group" id="2088af427587dee2" memberName="groupComponent5"
                  virtualName="" explicitFocusOrder="0" pos="8 312 736 392" title="Global"/>
  <GROUPCOMPONENT name="new group" id="c254958507a97525" memberName="groupComponent3"
                  virtualName="" explicitFocusOrder="0" pos="512 8 232 240" title="Direction"/>
  <GROUPCOMPONENT name="new group" id="34a6f2de8551577d" memberName="groupComponent2"
                  virtualName="" explicitFocusOrder="0" pos="8 8 232 240" title="Signal (dB)"/>
  <GROUPCOMPONENT name="" id="7f2fc093b46143e7" memberName="groupComponent" virtualName=""
                  explicitFocusOrder="0" pos="8 248 484 62" title="Host"/>
  <VIEWPORT name="" id="aaae2a98b6efee0" memberName="signalScope" virtualName=""
            explicitFocusOrder="0" pos="24 32 200 200" vscroll="0" hscroll="0"
            scrollbarThickness="18" contentType="1" jucerFile="scopeComponent.cpp"
            contentClass="" constructorParams=""/>
  <TEXTBUTTON name="" id="fa48314fd9db3fc6" memberName="connectButton" virtualName=""
              explicitFocusOrder="0" pos="352 264 120 32" bgColOff="ff808080"
              buttonText="Connect" connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <TEXTEDITOR name="new text editor" id="273225c6b138c9e8" memberName="textEditor"
              virtualName="" explicitFocusOrder="0" pos="24 264 320 32" initialText="linuxlightning.homedns.org"
              multiline="0" retKeyStartsLine="0" readonly="0" scrollbars="1"
              caret="1" popupmenu="1"/>
  <VIEWPORT name="" id="7887e52f246f7d33" memberName="directionscope" virtualName=""
            explicitFocusOrder="0" pos="528 32 200 200" vscroll="0" hscroll="0"
            scrollbarThickness="18" contentType="1" jucerFile="directionscopeComponent.cpp"
            contentClass="" constructorParams=""/>
  <GROUPCOMPONENT name="" id="164679686d8711aa" memberName="groupComponent4" virtualName=""
                  explicitFocusOrder="0" pos="260 8 232 240" title="Power (dB)"/>
  <VIEWPORT name="" id="cc0a53d6f31bccdf" memberName="powerScope" virtualName=""
            explicitFocusOrder="0" pos="276 32 200 200" vscroll="0" hscroll="0"
            scrollbarThickness="18" contentType="1" jucerFile="powerscopeComponent.cpp"
            contentClass="" constructorParams=""/>
  <TEXTBUTTON name="" id="bcf4f7b0888effe5" memberName="quitButton" virtualName=""
              explicitFocusOrder="0" pos="624 264 120 32" bgColOff="ff808080"
              buttonText="Quit" connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <VIEWPORT name="" id="ef0ad2f341c3b908" memberName="radarScreen" virtualName=""
            explicitFocusOrder="0" pos="16 328 720 360" vscroll="0" hscroll="0"
            scrollbarThickness="18" contentType="1" jucerFile="radarComponent.cpp"
            contentClass="" constructorParams=""/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif
