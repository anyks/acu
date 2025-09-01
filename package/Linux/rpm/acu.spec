Name:         @name@
Version:      @version@
Release:      @release_number@
Summary:      @summary@
License:      GPL-3.0
BuildArch:    @architecture@
Group:        Networking/Other
URL:          @url@
Distribution: @distribution@

# Устанавливаем каталог сборки
%define _rpmdir @work_path@

# Отключаем проверку RPATH
%global __brp_check_rpaths %{nil}

# Если сборка производится в Alt-linux
%if "%_vendor" == "alt"
   # Устанавливаем зависимость SCTP
   BuildRequires: liblksctp-devel

   # Отключаем проверку ELF зависимостей
   %set_verify_elf_method none

   # Отключаем проверку зависимостей
   %define __find_requires %{nil}
# Если сборка производится в другом дистрибутиве Linux
%else
   # Устанавливаем зависимость SCTP
   BuildRequires: lksctp-tools-devel
%endif

%description
@description@

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
