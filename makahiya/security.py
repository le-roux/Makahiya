from .models import Session, Users

GROUPS = {
    1: ['group:editors', 'group:sudo', 'group:viewers'],
    2: ['group:viewers']
}

def groupfinder(userid, request):
    user = Session().query(Users).filter_by(email=userid).first()
    return GROUPS.get(user.level, [])
