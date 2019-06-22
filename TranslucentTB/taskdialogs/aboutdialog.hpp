#pragma once
#include "taskdialog.hpp"
#include <iterator>
#include <rapidjson/rapidjson.h>
#include <sstream>
#include <wil/safecast.h>

#include "constants.hpp"
#include "../ExplorerDetour/hook.hpp"
#include "ttberror.hpp"
#include "util/numbers.hpp"
#include "../winrt/uwp.hpp"
#include "win32.hpp"

class AboutDialog : public TTBTaskDialog {
private:
	static constexpr int COPY_VERSION = 40000;
	static constexpr int JOIN_DISCORD = COPY_VERSION + 1;
	static constexpr int DONATE = JOIN_DISCORD + 1;

	static constexpr TASKDIALOG_BUTTON s_Buttons[] = {
		{
			COPY_VERSION,
			L"Copy system info to clipboard\nUse this when filling a GitHub bug report."
		},
		{
			JOIN_DISCORD,
			L"Join our Discord server\nChat with the community and developers."
		},
		{
			DONATE,
			L"Donate\nSupport us developing TranslucentTB and bringing other great features to you!"
		}
	};

	inline HRESULT CallbackProc(Window window, unsigned int uNotification, WPARAM wParam, LPARAM)
	{
		if (uNotification == TDN_BUTTON_CLICKED)
		{
			switch (wParam)
			{
			case COPY_VERSION:
				try
				{
					UWP::CopyToClipboard(BuildVersionInfo());
					MessageBox(window, L"Copied.", NAME, MB_OK | MB_ICONINFORMATION | MB_SETFOREGROUND);
				}
				WinrtExceptionCatch(Error::Level::Error, L"Failed to copy version information!")
				return S_FALSE;

			case JOIN_DISCORD:
				win32::OpenLink(L"https://discord.gg/w95DGTK");
				break;

			case DONATE:
				win32::OpenLink(L"https://liberapay.com/TranslucentTB");
				break;
			}
		}
		return S_OK;
	}

	inline static std::wstring BuildVersionInfo()
	{
		std::wostringstream str;

		str << L"Build configuration: "
#if defined(NDEBUG)
			L"Release"
#elif defined(_DEBUG)
			L"Debug"
#else
			L"Unknown"
#endif
			L" ("
#if defined(_AMD64_)
			L"x64"
#elif defined (_X86_)
			L"x86"
#elif defined(_ARM64_)
			L"ARM64"
#elif defined(_ARM_)
			L"ARM"
#else
			L"Unknown"
#endif
			<< L')' << std::endl;

		str << L"System architecture: " << win32::GetProcessorArchitecture() << std::endl;

		if (UWP::HasPackageIdentity())
		{
			str << L"Package version: ";
			try
			{
				str << UWP::GetApplicationVersion();
			}
			catch (const winrt::hresult_error &error)
			{
				str << error.message();
			}
			str << std::endl;
		}

		const auto [version, hr] = win32::GetFileVersion(win32::GetExeLocation());
		str << L"TranslucentTB version: " << (!version.empty() ? version : Error::ExceptionFromHRESULT(hr)) << std::endl;

		const auto [build, hr2] = win32::GetWindowsBuild();
		str << L"Windows version: " << (!build.empty() ? build : Error::ExceptionFromHRESULT(hr2)) << std::endl;

		const auto [major, minor, revision] = Hook::GetDetoursVersion();
		str << L"Microsoft Detours version: " << major << L'.' << minor << L'.' << revision << std::endl;

		str << L"RapidJSON version: " << RAPIDJSON_VERSION_STRING;

		return str.str();
	}

	inline static std::wstring BuildAboutContent()
	{
		std::wostringstream str;

		str << L"This program is free (as in freedom) software, redistributed under the GPLv3. ";
		str << LR"(As such, the <A HREF="https://github.com/TranslucentTB/TranslucentTB/">source code</A> is available for anyone to modify, inspect, compile, etc...)" << std::endl;
		str << LR"(Brought to you by <A HREF="https://github.com/TranslucentTB/TranslucentTB/graphs/contributors">all its contributors</A>.)" << std::endl << std::endl;
		str << BuildVersionInfo() << std::endl << std::endl;
		str << L"All trademarks, product names, company names, logos, service marks, copyrights and/or trade dress mentioned, displayed, cited, or otherwise indicated are the property of their respective owners.";

		return str.str();
	}

public:
	inline AboutDialog() :
		TTBTaskDialog(
			L"About " NAME,
			BuildAboutContent(),
			std::bind(&AboutDialog::CallbackProc, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
			Window::NullWindow
		)
	{
		m_Cfg.dwFlags |= TDF_ALLOW_DIALOG_CANCELLATION | TDF_USE_COMMAND_LINKS;
		m_Cfg.dwCommonButtons = TDCBF_CLOSE_BUTTON;

		m_Cfg.cButtons = wil::safe_cast<UINT>(std::size(s_Buttons));
		m_Cfg.pButtons = s_Buttons;
	}

	inline void Run()
	{
		bool _;
		TTBTaskDialog::Run(_);
	}
};