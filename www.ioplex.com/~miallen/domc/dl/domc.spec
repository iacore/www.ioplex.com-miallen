Summary: W3C's Document Object Model (DOM) in C.
Name: domc
Version: 0.8.0
Release: 1
Group: Development/Libraries
Source: http://www.ioplex.com/~miallen/domc/dl/domc-%{version}.tar.gz
URL: http://www.ioplex.com/~miallen/domc/
Copyright: MIT/X Consortium
Prefix: %{_prefix}
BuildRoot: %{_tmppath}/%{name}-root
Autoreq: 0
Requires: libmba
Packager: Michael B. Allen <mba2000@ioplex.com>

%description
DOMC is an implementation of the DOM in C as specified by various W3C
recommendations. DOMC may be used to manipulated XML documents as a
tree of nodes in memory. Optional serialization functions are provided
to load and store XML source files (requires Expat).

%prep
%setup -q

%build
make

%install
rm -rf $RPM_BUILD_ROOT
make install prefix=${RPM_BUILD_ROOT}%{_prefix} libdir=${RPM_BUILD_ROOT}%{_libdir} includedir=${RPM_BUILD_ROOT}%{_includedir}

%clean
rm -rf $RPM_BUILD_ROOT

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(-,root,root)
%doc README.txt docs
%{_libdir}/libdomc*
%{_includedir}/domc.h

%changelog
* Thu Sep  9 2004 Micheal B. Allen <mba2000 ioplex.com>
- 0.8.0

* Wed Aug  4 2004 Michael B. Allen <mba2000 ioplex.com>
- 0.7.1

* Sat Mar 22 2003 Michael B. Allen <mballen@erols.com>
- 0.7.0 

* Thu Mar 28 2002 Michael B. Allen <mballen@erols.com>
- Create.
