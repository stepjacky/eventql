/*
 * This file is part of the "libfnord" project
 *   Copyright (c) 2015 Paul Asmuth
 *
 * FnordMetric is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License v3.0. You should have received a
 * copy of the GNU General Public License along with this program. If not, see
 * <http://www.gnu.org/licenses/>.
 */
#ifndef _FNORD_FTS_STEMMER_H
#define _FNORD_FTS_STEMMER_H
#include "stx/stdtypes.h"
#include "stx/Language.h"

namespace stx {
namespace fts {

class Stemmer {
public:
  virtual ~Stemmer() {}
  virtual void stem(Language lang, stx::String* term) = 0;
};

} // namespace fts
} // namespace stx

#endif