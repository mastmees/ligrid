/*
  ==============================================================================

  This is an automatically generated file created by the Jucer!

  Creation date:  7 Jul 2009 10:23:59 pm

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Jucer version: 1.11

  ------------------------------------------------------------------------------

  The Jucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright 2004-6 by Raw Material Software ltd.

  ==============================================================================
*/

#ifndef __JUCER_HEADER_SCOPECOMPONENT_SCOPECOMPONENT_D89FA986__
#define __JUCER_HEADER_SCOPECOMPONENT_SCOPECOMPONENT_D89FA986__

//[Headers]     -- You can add your own extra header files here --
#include "../juce/juce.h"
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Jucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class scopeComponent  : public Component
{
public:
    //==============================================================================
    scopeComponent ();
    ~scopeComponent();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
	void SetData(int nsamples,int *data);
	bool DataChanged() { return needsrepainting!=0; }
    //[/UserMethods]

    void paint (Graphics& g);
    void resized();


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    //[UserVariables]   -- You can add your own custom variables in this section.
	int *SampleData;
	int SampleCount;
	int needsrepainting;
    //[/UserVariables]

    //==============================================================================


    //==============================================================================
    // (prevent copy constructor and operator= being generated..)
    scopeComponent (const scopeComponent&);
    const scopeComponent& operator= (const scopeComponent&);
};


#endif   // __JUCER_HEADER_SCOPECOMPONENT_SCOPECOMPONENT_D89FA986__
