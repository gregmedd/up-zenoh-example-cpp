// Copyright (c) 2024 General Motors GTO LLC
//
// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.
// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: 2024 General Motors GTO LLC
// SPDX-License-Identifier: Apache-2.0

#include <chrono>
#include <csignal>
#include <semaphore>
#include <spdlog/spdlog.h>
#include <up-client-zenoh-cpp/transport/ZenohUTransport.h>
#include <up-cpp/client/RpcInitiator.h>

#include "common.h"

std::binary_semaphore terminate{0};

void signalHandler(int signal) {
	if (signal == SIGINT) {
		spdlog::info("Ctrl+C received. Exiting...");
		terminate.release(); 
	}
}

// This sample RPC client demonstrates how to issue RPC requests and wait for a
// response back. In this example, the server will send back the current time
// as milliseconds since epoch.
int main() {
	signal(SIGINT, signalHandler);

	auto transport = uprotocol::transport::getTransport<ZenohUTransport>(
			defaultClientSource(), zenohCfgPath);

	if (nullptr == transport) {
		spdlog::error("ZenohUTransport init failed");
		return -1;
	}

	constexpr auto timeReqTtl = 20ms;

	RpcInitiator client;
	auto timeReqBuilder = client.requestBuilder(timeServerUri(), timeReqTtl);

	while (!terminate.try_acquire_for(1s)) {
		auto timeResponse = client.invokeMethod(timeReqBuilder.build());

		if (timeResponse.wait() != std::future_status::ready) {
			spdlog::error("Problem waiting for time future to ready");
			continue;
		}

		auto statusOrTime = timeResponse.get();
		if (std::holds_alternative<uprotocol::v1::UStatus>(statusOrTime)) {
			spdlog::error("Timeout waiting for time response");
			continue;
		}

		auto msg = std::get<uprotocol::v1::UMessage>(statusOrTime);
		spdlog::info("Received time = {}",
				TimeAsPayload::deserialize(msg.payload()));
	}

	return 0;
}
