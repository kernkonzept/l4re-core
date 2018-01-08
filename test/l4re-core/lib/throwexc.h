/*
 * Copyright (C) 2018 Kernkonzept GmbH.
 * Author(s): Marius Melzer <marius.melzer@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */

#pragma once

namespace Throw_exc
{

/**
 * Base class to be used as an exception.
 */
class Base_exc
{};

/**
 * Exception class to be thrown to test if the base class is caught.
 */
class Derived_exc : public Base_exc
{};

/**
 * Throws an exception of the base class.
 */
void throw_base_exc();

/**
 * Throws an exception of the visible derived class.
 */
void throw_derived_exc();

/**
 * Throws an exception of the derived class hidden within an anonymous
 * namespace.
 */
void throw_hidden_derived_exc();

/**
 * Throws a string.
 */
void throw_str_exc();

}
