Name:      @name@
Version:   @version@
Release:   1
Summary:   ANYKS - conversion utility (ACU)
License:   GPL-3.0
BuildArch: @architecture@
Group:     Networking/Other
URL:       https://acu.anyks.com

%description
Cross-platform utility for converting text formats - convert utility (ACU).

%define _rpmdir @work_path@

%install
mkdir -p $RPM_BUILD_ROOT/usr/bin
install @executable_file@ $RPM_BUILD_ROOT/usr/bin

%clean
%if "%{noclean}" == ""
   rm -rf $RPM_BUILD_ROOT
%endif

%files
%defattr(-,root,root)
/usr/bin/@package_name@
