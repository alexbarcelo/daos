#!/usr/bin/python
'''
  (C) Copyright 2018-2019 Intel Corporation.

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  GOVERNMENT LICENSE RIGHTS-OPEN SOURCE SOFTWARE
  The Government's rights to use, modify, reproduce, release, perform, display,
  or disclose this software are subject to the terms of the Apache License as
  provided in Contract No. B609815.
  Any reproduction of computer software, computer software documentation, or
  portions thereof marked with this legend must also reproduce the markings.
'''
from __future__ import print_function

import traceback
import ctypes
import agent_utils
import server_utils
import write_host_file
from pydaos.raw import DaosContext, DaosPool, DaosApiError, RankList
from apricot import TestWithoutServers, skipForTicket

class BadConnectTest(TestWithoutServers):
    """
    Tests pool connect calls passing NULL and otherwise inappropriate
    parameters.  This can't be done with daosctl, need to use the python API.
    :avocado: recursive
    """

    # start servers, establish file locations, etc.
    def setUp(self):
        super(BadConnectTest, self).setUp()
        self.agent_sessions = None
        self.hostlist_servers = None
        self.hostlist_servers = self.params.get("test_machines", '/run/hosts/')

        # NULL is causing connect to blow up so skip that test for now
        uuidlist = self.params.get("uuid", '/run/connecttests/UUID/*/')
        connectuuid = uuidlist[0]
        if connectuuid == 'NULLPTR':
            self.cancel("skipping null pointer test until DAOS-1781 is fixed")

        # launch the server
        self.hostfile_servers = write_host_file.write_host_file(
            self.hostlist_servers, self.workdir)
        server_group = self.params.get("name", '/server_config/',
                                       'daos_server')
        self.agent_sessions = agent_utils.run_agent(self,
                                                    self.hostlist_servers)
        server_utils.run_server(self, self.hostfile_servers, server_group)

    def tearDown(self):
        if self.agent_sessions:
            agent_utils.stop_agent(self.agent_sessions)
        server_utils.stop_server(hosts=self.hostlist_servers)

    @skipForTicket("DAOS-3819")
    def test_connect(self):
        """
        Pass bad parameters to pool connect

        :avocado: tags=all,pool,full_regression,tiny,badconnect
        """

        # parameters used in pool create
        createmode = self.params.get("mode", '/run/connecttests/createmode/')
        createuid = self.params.get("uid", '/run/connecttests/uids/createuid/')
        creategid = self.params.get("gid", '/run/connecttests/gids/creategid/')
        createsetid = self.params.get("setname",
                                      '/run/connecttests/setnames/createset/')
        createsize = self.params.get("size",
                                     '/run/connecttests/psize/createsize/')

        # Accumulate a list of pass/fail indicators representing what is
        # expected for each parameter then "and" them to determine the
        # expected result of the test
        expected_for_param = []

        modelist = self.params.get("mode", '/run/connecttests/connectmode/*/')
        connectmode = modelist[0]
        expected_for_param.append(modelist[1])

        svclist = self.params.get("ranklist", '/run/connecttests/svrlist/*/')
        svc = svclist[0]
        expected_for_param.append(svclist[1])

        setlist = self.params.get("setname",
                                  '/run/connecttests/connectsetnames/*/')
        connectset = setlist[0]
        expected_for_param.append(setlist[1])

        uuidlist = self.params.get("uuid", '/run/connecttests/UUID/*/')
        connectuuid = uuidlist[0]
        expected_for_param.append(uuidlist[1])

        # if any parameter is FAIL then the test should FAIL, in this test
        # virtually everyone should FAIL since we are testing bad parameters
        expected_result = 'PASS'
        for result in expected_for_param:
            if result == 'FAIL':
                expected_result = 'FAIL'
                break

        puuid = (ctypes.c_ubyte * 16)()
        psvc = RankList()
        pgroup = ctypes.create_string_buffer(0)
        pool = None
        try:
            # initialize a python pool object then create the underlying
            # daos storage
            pool = DaosPool(self.context)
            pool.create(createmode, createuid, creategid,
                        createsize, createsetid, None)
            # save this uuid since we might trash it as part of the test
            ctypes.memmove(puuid, pool.uuid, 16)

            # trash the the pool service rank list
            psvc.rl_ranks = pool.svc.rl_ranks
            psvc.rl_nr = pool.svc.rl_nr
            if not svc == 'VALID':
                rl_ranks = ctypes.POINTER(ctypes.c_uint)()
                pool.svc = RankList(rl_ranks, 1)

            # trash the pool group value
            pgroup = pool.group
            if connectset == 'NULLPTR':
                pool.group = None

            # trash the UUID value in various ways
            if connectuuid == 'NULLPTR':
                pool.uuid = None
            if connectuuid == 'JUNK':
                pool.uuid[4] = 244

            pool.connect(connectmode)

            if expected_result in ['FAIL']:
                self.fail("Test was expected to fail but it passed.\n")

        except DaosApiError as excep:
            print(excep)
            print(traceback.format_exc())
            if expected_result in ['PASS']:
                self.fail("Test was expected to pass but it failed.\n")

        # cleanup the pool
        finally:
            if pool is not None and pool.attached == 1:
                # restore values in case we trashed them during test
                pool.svc.rl_ranks = psvc.rl_ranks
                pool.svc.rl_nr = psvc.rl_nr
                pool.group = pgroup
                ctypes.memmove(pool.uuid, puuid, 16)
                print("pool uuid after restore {}".format(
                    pool.get_uuid_str()))
                pool.destroy(1)
