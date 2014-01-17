from __future__ import print_function

import re

import tables

_table_names = {"agents": "Agents",}

_agent_key = "ID"
_agent_schema = ["AgentType", "ModelType", "Prototype", "ParentID", "EnterDate"]

_agent_deaths_key = "AgentID"
_agent_deaths_schema = ["DeathDate"]

_simulation_time_info_schema = ["InitialYear", "InitialMonth", "SimulationStart",
                                "Duration", "DecayInterval"]

_agent_id_names = ["ParentID"]

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

    def __del__(self):
        self._db.close()
        
    def _populate_agent_invariants(self):
        invars = {}
        table = self._db.get_node(self._db.root,
                                  name = _table_names["agents"], 
                                  classname = "Table")
        for row in table.iterrows():
            a_id = row["ID"]
            p_id = row["ParentID"]
            p_invar = None
            if (a_id != p_id):
                p_invar = invars[p_id]
            invars[a_id] = tuple(row[i] if i not in _agent_id_names else p_invar 
                                 for i in _agent_schema)
        return invars
    
    def walk(self):
        """Visits all tables, populating an equality-comparable object
        """
        ret = set()
        for table in self._db.walk_nodes(classname = "Table"):
            methname = 'visit' + re.sub('([A-Z]+)', r'_\1', table._v_name).lower()
            if hasattr(self, methname):
                print(methname)
                meth = getattr(self, methname)
                obj = meth(table)
                ret.add(obj)
        return ret

    def visit_agents(self, table):
        d = {self.agent_invariants[row[_agent_key]]:
                 tuple(row[i] if i not in _agent_id_names
                       else self.agent_invariants[row[i]] 
                       for i in _agent_schema)
             for row in table.iterrows()}
        return tuple((k, d[k]) for k in sorted(d.keys()))

    def visit_agent_deaths(self, table):
        d = {self.agent_invariants[row[_agent_deaths_key]]:
                 tuple(row[i] if i not in _agent_id_names
                       else self.agent_invariants[row[i]] 
                       for i in _agent_deaths_schema)
             for row in table.iterrows()}
        return tuple((k, d[k]) for k in sorted(d.keys()))

    def visit_simulation_time_info(self, table):
        return tuple(row[i] for i in _simulation_time_info_schema
                     for row in table.iterrows())
