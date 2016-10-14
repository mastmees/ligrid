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

#ifndef __JUCER_HEADER_MAINCOMPONENT_MAINCOMPONENT_AB6A065B__
#define __JUCER_HEADER_MAINCOMPONENT_MAINCOMPONENT_AB6A065B__

//[Headers]     -- You can add your own extra header files here --
#include "../juce/juce.h"
#include "../ipnetwork.hpp"
#include "ConnectionThread.h"
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Jucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class MainComponent  : public Component,
                       public Timer,
                       public ButtonListener
{
public:
    //==============================================================================
    MainComponent ();
    ~MainComponent();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
	virtual void timerCallback();
    //[/UserMethods]

    void paint (Graphics& g);
    void resized();
    void buttonClicked (Button* buttonThatWasClicked);


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    //[UserVariables]   -- You can add your own custom variables in this section.
	CTCPConnection connection;
    //[/UserVariables]

    //==============================================================================
    GroupComponent* groupComponent5;
    GroupComponent* groupComponent3;
    GroupComponent* groupComponent2;
    GroupComponent* groupComponent;
    Viewport* signalScope;
    TextButton* connectButton;
    TextEditor* textEditor;
    Viewport* directionscope;
    GroupComponent* groupComponent4;
    Viewport* powerScope;
    TextButton* quitButton;
    Viewport* radarScreen;

    //==============================================================================
    // (prevent copy constructor and operator= being generated..)
    MainComponent (const MainComponent&);
    const MainComponent& operator= (const MainComponent&);
};


#endif   // __JUCER_HEADER_MAINCOMPONENT_MAINCOMPONENT_AB6A065B__
