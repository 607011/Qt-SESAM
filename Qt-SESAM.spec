Name:           Qt-SESAM
Version:        2.0.6
Release:        1%{?dist}
Summary:        Super Easy & Secure Authentication Management

Group:          Applications/Internet
License:        GPLv3+ and Boost
URL:            https://github.com/ola-ct/%{name}
Source0:        https://github.com/ola-ct/%{name}/archive/v%{version}.tar.gz

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
BuildRequires:  qtkeychain-devel


# @TODO: we need a better description text
%description
Super Easy & Secure Authentication Management

This program uses the Crypto++ library.


%prep
%setup -q -n %{name}-%{version}

%build
lrelease-qt5 %{name}/%{name}.pro
qmake-qt5 %{name}.pro CONFIG+=system_qtkeychain
make %{?_smp_mflags}
make %{?_smp_mflags} check

%install
%{make_install} INSTALL_ROOT=%{buildroot}
%{find_lang} QtSESAM --with-qt --without-mo

%clean
rm -rf %{buildroot}


%files -f QtSESAM.lang
%defattr(-,root,root,-)
%doc README.md
%license LICENSE libSESAM/3rdparty/cryptopp/Crypto++-License
%{_bindir}/%{name}


%changelog
* Mon Sep 28 2015 Oliver Lau <ola@ct.de> - 2.0.6-1
- update to 2.0.6

* Fri Sep 18 2015 Stefan Becker <chemobejk@gmail.com> - 2.0.5-1
- update to 2.0.5

* Wed Sep 16 2015 Stefan Becker <chemobejk@gmail.com> - 2.0.4-1
- update to 2.0.4

* Tue Sep 15 2015 Stefan Becker <chemobejk@gmail.com> - 2.0.3-1
- Initial packaging.
