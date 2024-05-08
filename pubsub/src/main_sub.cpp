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

#include <csignal>
#include <spdlog/spdlog.h>
#include <up-client-zenoh-cpp/transport/ZenohUTransport.h>

#include "common.h"

bool gTerminate = false;

void signalHandler(int signal) {
	if (signal == SIGINT) {
		spdlog::info("Ctrl+C received. Exiting...");
		gTerminate = true;
	}
}

template<typename PayloadT>
void receive(std::string_view name, const uprotocol::v1::UMessage& message) {
	if (message.payload.size() < sizeof(PayloadT)) {
		spdlog::error("Payload too small ({}) for \"{}\"",
				message.payload().size(), name);
		return;
	}

	const PayloadT *value =
		reinterpret_cast<const PayloadT*>(message.payload().data());
	spdlog::info("{} = {}", name, *value);
}

/* The sample sub applications demonstrates how to consume data using uTransport -
 * There are three topics that are received - random number, current time and a counter */
int main() {
	signal(SIGINT, signalHandler);

	auto transport = uprotocol::transport::getTransport<ZenohUTransport>(
			defaultSubscriberUri(), zenohConfig);

	if (nullptr == transport){
		spdlog::error("ZenohUTransport init failed");
		return -1;
	}

	using namespace uprotocol::client;

	auto [timeStatus, timeHandle] = Subscriber::subscribe(
			transport, getTimeUri(), [](const uprotocol::v1::UMessage& msg) {
				receive("time", msg);
			});
	auto [randStatus, randHandle] = Subscriber::subscribe(
			transport, getRandomUri(), [](const uprotocol::v1::UMessage& msg) {
				receive("random", msg);
			});
	auto [countStatus, countHandle] = Subscriber::subscribe(
			transport, getCountUri(), [](const uprotocol::v1::UMessage& msg) {
				receive("counter", msg);
			});

	while (!gTerminate) {
		std::this_thread::sleep_for(1s);
	}

	return 0;
}
