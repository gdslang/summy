/*
 * shared_copy.h
 *
 *  Created on: Mar 16, 2015
 *      Author: Julian Kranz
 */

#pragma once

#include <memory>
#include <cppgdsl/rreil/id/id.h>

std::shared_ptr<gdsl::rreil::id> shared_copy(gdsl::rreil::id const *id);
