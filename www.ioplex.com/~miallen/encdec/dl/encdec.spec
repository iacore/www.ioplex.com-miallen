Summary: Encode and decode a wide range of C objects.
Name: encdec
Version: 0.4.0
Release: 1
Group: Development/Libraries
Source: http://www.ioplex.com/~miallen/encdec/dl/encdec-%{version}.tar.gz
URL: http://www.ioplex.com/~miallen/encdec/
Copyright: LGPL
Prefix: %{_prefix}
BuildRoot: %{_tmppath}/%{name}-root
Autoreq: 0
Packager: Michael B. Allen <mballen@erols.com>

%description
This C library may be used to encode and decode C objects such  as
integers, floats, doubles, times, and internationalized strings to and
from a wide variety of binary formats as they might appear in various
file formats or network messages.

%prep
%setup -q

%build
make LIBICONV=/home/miallen/p/c/libiconv-1.7

%install
rm -rf $RPM_BUILD_ROOT
make install prefix=${RPM_BUILD_ROOT}%{_prefix} libdir=${RPM_BUILD_ROOT}%{_libdir} includedir=${RPM_BUILD_ROOT}%{_includedir} mandir=${RPM_BUILD_ROOT}%{_mandir}

%clean
rm -rf $RPM_BUILD_ROOT

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(-,root,root)
%doc README.txt docs
%{_libdir}/libencdec*
%{_includedir}/encdec.h
%{_mandir}/man3/encdec.3m.gz

%changelog
* Mon Jul 22 2003 Michael B. Allen <mballen@erols.com>
- 0.4.0

* Sat Mar 22 2003 Michael B. Allen <mballen@erols.com>
- 0.3.7

* Sat Nov 16 2002 Michael B. Allen <mballen@erols.com>
- 0.3.6

* Thu Mar 28 2002 Michael B. Allen <mballen@erols.com>
- Create.
