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
mkdir -p %{buildroot}/usr/bin
install @executable_file@ %{buildroot}/usr/bin

%clean
cp $(find @work_path@ -name "@name@*.rpm") %{buildroot}/../
%if "%{noclean}" == ""
   rm -rf %{buildroot}
%endif

%files
%defattr(-,root,root,-)
/usr/bin/@name@

%changelog
* @date@ @distribution@ <@email@> - @version@-@release_number@
- Conversion utility version v@version@-@release_number@ release
