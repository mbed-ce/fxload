/*
 * Copyright (c) 2007 Claudio favi (claudio.favi@epfl.ch)
 * Copyright (c) 2001 Stephen Williams (steve@icarus.com)
 * Copyright (c) 2001-2002 David Brownell (dbrownell@users.sourceforge.net)
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#ifndef FXLOAD_APPLICATIONPATHS_H
#define FXLOAD_APPLICATIONPATHS_H

#include <string>

// Basic utilities for finding the executable's path (used to find the resource directory)
// This code from https://stackoverflow.com/a/60250581/7083698
namespace AppPaths {

  std::string getExecutablePath();
  std::string getExecutableDir();

#if defined(_WIN32)
  static const std::string PATH_SEP = "\\";
#else
  static const std::string PATH_SEP = "/";
#endif
}


#endif //FXLOAD_APPLICATIONPATHS_H
