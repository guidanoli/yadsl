from pygraph import Graph

dtypes = [42, True, 'str', None]

def test_directed():
    g = Graph(directed=True)
    assert g.is_directed()

def test_undirected():
    g = Graph(directed=False)
    assert not g.is_directed()

def test_add_remove_vertex():
    def _test(g, o):
        c = g.get_vertex_count()
        g.add_vertex(o)
        assert g.contains_vertex(o)
        assert g.get_vertex_count() == c + 1
        g.remove_vertex(o)
        assert not g.contains_vertex(o)
        assert g.get_vertex_count() == c
    g = Graph()
    for dtype in dtypes:
        _test(g, dtype)

def test_recursion():
    g = Graph()
    class X:
        def __eq__(self, o):
            g.add_vertex(self)
    g.add_vertex(0)
    try:
        g.add_vertex(X())
    except RecursionError as e:
        return
    assert False

def test_add_remove_edge():
    def _test(g, u, v, e):
        g.add_edge(u,v,e)
        assert g.contains_edge(u,v)
        assert g.get_edge(u,v) == e
        g.remove_edge(u,v)
        assert not g.contains_edge(u,v)
    g = Graph()
    for dtype in dtypes:
        g.add_vertex(dtype)
    for d1 in dtypes:
        for d2 in dtypes:
            _test(g, d1, d2, (d1, d2))

def test_vertices():
    g = Graph()
    for i in range(10):
        assert len(g.vertices()) == g.get_vertex_count()
        g.add_vertex(i)
        assert i in g.vertices()
    for i in range(10):
        g.remove_vertex(i)
        assert len(g.vertices()) == g.get_vertex_count()
        assert i not in g.vertices()

def test_neighbours():
    g = Graph()
    for i in range(10):
        g.add_vertex(i)
    for v in g.vertices():
        for w in range(v):
            assert len(g.neighbours(v)) == g.degree(v)
            assert len(g.neighbours(v, outgoing = False)) == g.degree(v, outgoing = False)
            assert len(g.neighbours(v, ingoing = False)) == g.degree(v, ingoing = False)
            assert g.degree(v) == g.degree(v, outgoing = False) + g.degree(v, ingoing = False)
            g.add_edge(v,w,v-w)
            assert (w,v-w) in g.neighbours(v)
            assert (v,v-w) in g.neighbours(w)
            assert (w,v-w) in g.neighbours(v, ingoing = False)
            assert (v,v-w) in g.neighbours(w, outgoing = False)
            assert (w,v-w) not in g.neighbours(v, outgoing = False)
            assert (v,v-w) not in g.neighbours(w, ingoing = False)

def test_flags():
    g = Graph()
    g.add_vertex(0)
    g.set_flag(0,0)
    assert g.get_flag(0) == 0
    g.add_vertex(1)
    g.set_flag(1,1)
    assert g.get_flag(0) == 0
    assert g.get_flag(1) == 1
    g.set_all_flags(2)
    assert g.get_flag(0) == 2
    assert g.get_flag(1) == 2
    g.set_flag(0,3)
    assert g.get_flag(0) == 3
    assert g.get_flag(1) == 2