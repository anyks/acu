Name:         @name@
Version:      @version@
Release:      @release_number@
Summary:      @summary@
License:      GPL-3.0
BuildArch:    @architecture@
Group:        Networking/Other
URL:          @url@
Distribution: @distribution@

%description
@description@

%define _rpmdir @work_path@

%define current_date %(LC_ALL=en_US.UTF-8 date +"%a %h %d %Y")

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

%changelog
* %{current_date} @distribution@ <@email@> - @version@-@release_number@
- Conversion utility version v@version@-@release_number@ release
