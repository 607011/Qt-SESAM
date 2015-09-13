Name:           Qt-SESAM
Version:        2.0.3
Release:        1%{?dist}
Summary:        Super Easy & Secure Authentication Management

# @TODO: is this a legal value for the group tag?
Group:          Applications/Internet
License:        GPLv3+ and Boost
URL:            https://github.com/ola-ct/Qt-SESAM
Source0:        https://github.com/ola-ct/%{name}/archive/v%{version}-RELEASE.tar.gz
Patch0:         Qt-SESAM-2.0.3-fix-hash-master-build.patch
Patch1:         Qt-SESAM-2.0.3-unit-tests-as-console-app.patch
Patch2:         Qt-SESAM-2.0.3-automate-translations-build.patch

# build uses qmake, i.e. doesn't use information from pkconfig() or cmake()
# @TODO: what is correct BR for qmake-qt5?
BuildRequires:  pkgconfig(Qt5)
BuildRequires:  pkgconfig(Qt5Concurrent)
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Gui)
BuildRequires:  pkgconfig(Qt5Network)
BuildRequires:  pkgconfig(Qt5Widgets)
# for lrelease-qt5
BuildRequires:  cmake(Qt5LinguistTools)
BuildRequires:  dos2unix


# @TODO: we need a better description text
%description
Super Easy & Secure Authentication Management

This program uses the Crypto++ library.


%prep
# github automatic tarball generation directory naming scheme
%autosetup -n %{name}-%{version}-RELEASE -p1

%build
qmake-qt5 QtSESAM.pro
make %{?_smp_mflags}
make %{?_smp_mflags} check

# fixup DOS line endings
dos2unix README.md

%install
mkdir -p %{buildroot}%{_bindir}
cp -p ctSESAM/QtSESAM %{buildroot}%{_bindir}/

%clean
rm -rf %{buildroot}


%files
%defattr(-,root,root,-)
%doc README.md
%license LICENSE cryptopp/License.txt
%{_bindir}/QtSESAM


%changelog
* Sat Sep 12 2015 Stefan Becker <chemobejk@gmail.com> - 2.0.3-1
- Initial packaging.
