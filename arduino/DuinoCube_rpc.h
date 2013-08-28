// Copyright (C) 2013 Simon Que
//
// This file is part of ChronoCube.
//
// ChronoCube is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ChronoCube is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with ChronoCube.  If not, see <http://www.gnu.org/licenses/>.

// DuinoCube Remote Procedure Call (RPC) definitions.

#ifndef __DUINOCUBE_RPC_H___
#define __DUINOCUBE_RPC_H__

// NOTE: These should match the verilog definitions in the chronocube repo.

// RPC Client (Arduino) statuses.
#define MCU_RPC_NONE              0
#define MCU_RPC_ISSUED            1
#define MCU_RPC_WAITING           2

// RPC Server (Coprocessor) statuses.
#define COP_RPC_POWER_ON          0
#define COP_RPC_READY             1
#define COP_RPC_RECEIVED          2
#define COP_RPC_DONE              3

#endif  // __DUINOCUBE_RPC_H__
