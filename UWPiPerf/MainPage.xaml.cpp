﻿//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#include "pch.h"
#include "MainPage.xaml.h"
#include "WiFiServer.xaml.h"
#include "WiFiClient.xaml.h"

#include "socket_threads.h"

using namespace SDKTemplate;
using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::UI::Xaml::Interop;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

MainPage^ MainPage::Current = nullptr;
Object^ MainPage::ChildFrame = nullptr;
FrameType MainPage::ChildType = FrameType::WiFiClient;

MainPage::MainPage()
{
    InitializeComponent();
    SampleTitle->Text = FEATURE_NAME;

    // This is a static public property that allows downstream pages to get a handle to the MainPage instance
    // in order to call methods that are in this class.
    MainPage::Current = this;

	// Init the parameter for socket thread
	main_parameters_init();
}

MainPage::~MainPage()
{
	// Init the parameter for socket thread
	main_parameters_release();
}

void MainPage::OnNavigatedTo(NavigationEventArgs^ e)
{
    // Populate the ListBox with the scenarios as defined in Configuration.cpp.
    auto itemCollection = ref new Platform::Collections::Vector<Object^>();
    int i = 1;
    for (auto const& s : MainPage::Current->scenarios)
    {
        // Create a textBlock to hold the content and apply the ListItemTextStyle from Styles.xaml
        TextBlock^ textBlock = ref new TextBlock();
        ListBoxItem^ item = ref new ListBoxItem();
        auto style = App::Current->Resources->Lookup("ListItemTextStyle");

        textBlock->Text = (i++).ToString() + ") " + s.Title;
        textBlock->Style = safe_cast<Windows::UI::Xaml::Style ^>(style);

        item->Name = s.ClassName;
        item->Content = textBlock;
        itemCollection->Append(item);
    }

    // Set the newly created itemCollection as the ListBox ItemSource.
    ScenarioControl->ItemsSource = itemCollection;
    int startingScenarioIndex;

    if (Window::Current->Bounds.Width < 640)
    {
        startingScenarioIndex = -1;
    }
    else
    {
        startingScenarioIndex = 0;
    }

    ScenarioControl->SelectedIndex = startingScenarioIndex;
    ScenarioControl->ScrollIntoView(ScenarioControl->SelectedItem);
}


void MainPage::ScenarioControl_SelectionChanged(Object^ sender, SelectionChangedEventArgs^ e)
{
    ListBox^ scenarioListBox = safe_cast<ListBox^>(sender); //as ListBox;
    ListBoxItem^ item = dynamic_cast<ListBoxItem^>(scenarioListBox->SelectedItem);
    if (item != nullptr)
    {
        // Clear the status block when changing scenarios
        NotifyUser("", NotifyType::StatusMessage);

        // Navigate to the selected scenario.
        TypeName scenarioType = { item->Name, TypeKind::Custom };
        ScenarioFrame->Navigate(scenarioType, this);

        if (Window::Current->Bounds.Width < 640)
        {
            Splitter->IsPaneOpen = false;
        }
    }
}

void MainPage::NotifyUser(String^ strMessage, NotifyType type)
{
    switch (type)
    {
    case NotifyType::StatusMessage:
        StatusBorder->Background = ref new SolidColorBrush(Windows::UI::Colors::Green);
        break;
    case NotifyType::ErrorMessage:
        StatusBorder->Background = ref new SolidColorBrush(Windows::UI::Colors::Red);
        break;
	case NotifyType::ReportMessage:
		switch (ChildType)
		{
		case FrameType::WiFiServer:
			if (ChildFrame != nullptr)
			{
				//static_cast<SDKTemplate::WiFiTestTools::WiFiServer^>(ChildFrame)
				SDKTemplate::WiFiTestTools::WiFiServer^ server;

				server = static_cast<SDKTemplate::WiFiTestTools::WiFiServer^>(ChildFrame);
				
				server->AppendReportMessage(strMessage);
			}
			break;;
		case FrameType::WiFiClient:
			if (ChildFrame != nullptr)
			{
				//static_cast<SDKTemplate::WiFiTestTools::WiFiServer^>(ChildFrame)
				SDKTemplate::WiFiTestTools::WiFiClient^ client;

				client = static_cast<SDKTemplate::WiFiTestTools::WiFiClient^>(ChildFrame);

				client->AppendReportMessage(strMessage);
			}
			break;;
		default:
			break;
		}
		
		return;
	case NotifyType::IperfThreadEnd:
		switch (ChildType)
		{
		case FrameType::WiFiServer:
			if (ChildFrame != nullptr)
			{
				//static_cast<SDKTemplate::WiFiTestTools::WiFiServer^>(ChildFrame)
				SDKTemplate::WiFiTestTools::WiFiServer^ server;

				server = static_cast<SDKTemplate::WiFiTestTools::WiFiServer^>(ChildFrame);

				server->IperfThreadEnd();
			}
			break;;
		case FrameType::WiFiClient:
			if (ChildFrame != nullptr)
			{
				//static_cast<SDKTemplate::WiFiTestTools::WiFiServer^>(ChildFrame)
				SDKTemplate::WiFiTestTools::WiFiClient^ client;

				client = static_cast<SDKTemplate::WiFiTestTools::WiFiClient^>(ChildFrame);

				client->IperfThreadEnd();
			}
			break;;
		default:
			break;
		}
		return;
    default:
        break;
    }
    StatusBlock->Text = strMessage;

    // Collapse the StatusBlock if it has no text to conserve real estate.
    if (StatusBlock->Text != "")
    {
        StatusBorder->Visibility = Windows::UI::Xaml::Visibility::Visible;
        StatusPanel->Visibility = Windows::UI::Xaml::Visibility::Visible;
    }
    else
    {
        StatusBorder->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
        StatusPanel->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
    }
}

void MainPage::Footer_Click(Object^ sender, RoutedEventArgs^ e)
{
    auto uri = ref new Uri((String^)((HyperlinkButton^)sender)->Tag);
    Windows::System::Launcher::LaunchUriAsync(uri);
}

void MainPage::Button_Click(Object^ sender, RoutedEventArgs^ e)
{
    Splitter->IsPaneOpen = !Splitter->IsPaneOpen;
}

void MainPage::ScenarioItemEnable(int index, bool enable)
{
	if (index >= 2)
		return;

	ListBoxItem^ item = dynamic_cast<ListBoxItem^>(ScenarioControl->Items->GetAt(index));

	item->IsEnabled = enable;
}