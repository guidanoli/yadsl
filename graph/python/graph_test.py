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
        assert o in g
        assert g.get_vertex_count() == c + 1
        g.remove_vertex(o)
        assert o not in g
        assert g.get_vertex_count() == c
    g = Graph()
    for dtype in dtypes:
        _test(g, dtype)

def test_recursion():
    g = Graph()
    class X:
        def __eq__(self, o):
            g.add_vertex(self)
    try:
        g.add_vertex(0)
        g.add_vertex(X())
    except RecursionError as e:
        return
    assert False

def test_add_remove_edge():
    def _test(g, u, v, e):
        g.add_edge(u,v,e)
        assert g.get_edge(u,v) == e
        g.remove_edge(u,v)
    g = Graph()
    for dtype in dtypes:
        g.add_vertex(dtype)
    for d1 in dtypes:
        for d2 in dtypes:
            _test(g, d1, d2, (d1, d2))