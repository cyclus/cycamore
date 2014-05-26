from __future__ import print_function

import re
from collections import defaultdict

import numpy as np
import tables

_invar_table_names = {"agents": "AgentEntry",
                      "rsrcs": "Resources"}

_agent_key = "AgentId"
_agent_schema = ["Kind", "Implementation", "Prototype", "ParentId", "EnterTime"]

_simulation_time_info_schema = ["InitialYear", "InitialMonth", 
                                "Duration"]

_xaction_schema = ["SenderId", "ReceiverId", "ResourceId", "Commodity", 
                   "Time"]

_rsrc_key = "ResourceId"
_rsrc_schema = ["Type", "TimeCreated", "Quantity", "Units"]

_agent_id_names = ["ParentId", "SenderId", "ReceiverId"]
_rsrc_id_names = ["ResourceId"]

class HDF5RegressionVisitor(object):
    """ An HDF5RegressionVisitor visits a number of Cyclus HDF5 tables,
    returning objects that can be equality-comparable with other visitors.
    """

    def __init__(self, db_path):
        """Parameters
        ----------
        db_path : str
           The path to an HDF5 database
        """
        self._db = tables.open_file(db_path, mode = "r")
        self.agent_invariants = self._populate_agent_invariants()
        self.rsrc_invariants = self._populate_rsrc_invariants()

    def __del__(self):
        self._db.close()
        
    def _populate_agent_invariants(self):
        invars = {}
        table = self._db.get_node(self._db.root,
                                  name=_invar_table_names["agents"],
                                  classname="Table")
        for row in table.iterrows():
            a_id = row["AgentId"]
            p_id = row["ParentId"]
            p_invar = None
            # print(p_id, a_id)
            if p_id != -1:
                if p_id not in invars:
                    raise KeyError("Parent with id " + str(a_id) +\
                                       " not previously registered.")
                else:
                    p_invar = invars[p_id]
            newrow = []
            for i in _agent_schema:
                if i in _agent_id_names:
                    newrow.append(p_invar)
                elif isinstance(row[i], np.ndarray):
                    newrow.append(tuple(row[i]))
                else:
                    newrow.append(row[i])
            invars[a_id] = tuple(newrow)
        return invars
    
    def _populate_rsrc_invariants(self):
        table = self._db.get_node(self._db.root,
                                  name = _invar_table_names["rsrcs"], 
                                  classname = "Table")
        d = {}
        for row in table.iterrows():
            entry = []
            for item in _rsrc_schema:
                if isinstance(row[item], np.ndarray):
                    entry.append(tuple(row[item]))
                else:
                    entry.append(row[item])
            d[row[_rsrc_key]] = tuple(entry)
        return d

    def _xaction_entry(self, row):
        entry = []
        for item in _xaction_schema:
            if item in _agent_id_names:
                entry.append(self.agent_invariants[row[item]])
            elif item in _rsrc_id_names:
                entry.append(self.rsrc_invariants[row[item]])
            else:
                entry.append(row[item])
        return tuple(tuple(i) if isinstance(i, np.ndarray) else i for i in entry)
    
    def walk(self):
        """Visits all tables, populating an equality-comparable object
        """
        ret = set()
        for table in self._db.walk_nodes(classname = "Table"):
            tblname = re.sub('([A-Z]+)', r'\1', table._v_name).lower()
            methname = 'visit_' + tblname
            if hasattr(self, methname):
                print("visiting ", tblname)
                meth = getattr(self, methname)
                obj = meth(table)
                ret.add(obj)
        return ret

    def visit_agententry(self, table):
        d = {}
        for row in table.iterrows():
            entry = []
            for i in _agent_schema:
                if i in _agent_id_names:
                    entry.append(self.agent_invariants[row[_agent_key]])
                elif isinstance(row[i], np.ndarray):
                    entry.append(tuple(row[i]))
                else:
                    entry.append(row[i])
            d[self.agent_invariants[row[_agent_key]]] = tuple(entry)
        return tuple((k, d[k]) for k in sorted(d.keys()))

    def visit_info(self, table):
        return tuple(row[i] for i in _simulation_time_info_schema
                     for row in table.iterrows())

    def visit_transactions(self, table):
        xactions = []
        tmin = table[0]["Time"]
        tmax = table[-1]["Time"]
        xactions = []
        for i in range(tmin, tmax + 1):
            entry = set()
            for row in table.where('Time ==' + str(i)):
                entry.add(self._xaction_entry(row))
            xactions.append(frozenset(entry))
        return tuple(xactions)
