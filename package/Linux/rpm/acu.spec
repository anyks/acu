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
* @date@ @distribution@ <@email@> - @version@-@release_number@
- Conversion utility version v@version@-@release_number@ release
