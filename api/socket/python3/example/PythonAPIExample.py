from GstoreConnector import GstoreConnector
import sys
sys.path.append('../src')
# print(sys.path)

if __name__ == '__main__':
    gc = GstoreConnector('127.0.0.1', 3305)
    gc.build('LUBM10', 'data/lubm/lubm.nt')
    gc.load('LUBM10')

    sparql = '''select ?x where {
        ?x    <rdf:type>    <ub:UndergraduateStudent>.
        ?y    <ub:name> <Course1>.
        ?x    <ub:takesCourse>  ?y.
        ?z    <ub:teacherOf>    ?y.
        ?z    <ub:name> <FullProfessor1>.
        ?z    <ub:worksFor>    ?w.
        ?w    <ub:name>    <Department0>.
    }'''
    answer = gc.query(sparql)
    print(answer)

    gc.unload('LUBM10')
    # gc.load('LUBM10')
    # answer = gc.query(sparql)
    # print(answer)
    # gc.unload('LUBM10')
