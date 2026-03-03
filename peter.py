import random,time,re,sys,math
from datetime import date,timedelta
from pprint import pprint
from collections import defaultdict
ENG=0
RUS=1
LANG=ENG
if len(sys.argv)>1 and sys.argv[1][:2].lower()=='ru':
 LANG=RUS
def T(x):
 x=x.split('|')
 try:
  return x[LANG]
 except IndexError:
  return x[0]
gender_pattern=re.compile('([^@]*)@([A-Za-zА-Яа-яЁё]*)(.*)',re.DOTALL)
def detect_case(s):
 if s.islower():
  return 'lower'
 if s.istitle():
  return 'title'
 if s.isupper():
  return 'upper'
 return 'random'
def turn_to_case(s,case):
 if case=='lower':
  return s.lower()
 if case=='title':
  return s.title()
 if case=='upper':
  return s.upper()
 return ''.join([random.choice(c) for c in zip(s.lower(),s.upper())])
sim_filters={
'merchants':lambda x:'merchant' in x.traits.get('job',''),
}
wins=defaultdict(int)
losses=defaultdict(int)
research=defaultdict(float)
loyalty_control=defaultdict(float)
quality_control=defaultdict(float)
turn=0
def get_year():
 b,a=divmod(turn,365*4+1)
 b*=4
 c,d=divmod(a,365)
 if c>3:
  return b+3,a-365*3
 return b+c,d
def get_mean(c):
 n=0.
 a=0
 c=c+':'
 for k,v in research.items():
  if k.startswith(c):
   n+=v
   a+=1
 if not a:
  return 1
 return n/a
def research_turn():
 global turn
 turn+=1
 if get_year()[0]==2012:
  turn=0
 year,day=get_year()
 mean_freq=get_mean('freq')
 mean_loyalty=get_mean('happy')
 for k,v in research.items():
  if k.startswith('happy:') and v!=mean_loyalty:
   n=k[6:]
   loyalty_control[n]=min(max(loyalty_control[n]+(v-mean_loyalty)*.0001,
    -1e5),1e10)
  elif k.startswith('freq:') and v!=mean_freq:
   n=k[5:]
   quality_control[n]=min(max(quality_control[n]+(v-mean_freq)*
    (math.sin(day*(math.pi*2/366))*.01),-1e10),1e10)
 for k in research.keys():
  research[k]*=.999
def record(q,v):
 research[q]+=(v-research[q])*.01
loc_aliases={
'Earth':('home',T('Peter|Питер')),
'Alpha Centauri':('home',T('Zorloc|Зорлок')),
}
space_locs=['space','Earth','Alpha Centauri']
known_genders={
'оно':{
 '':'о',
},
'he':{
 'it':'he',
 'its':'his',
 "оно":"он",
},
'she':{
 'it':'she',
 'its':'her',
 '':'а',
 "оно":"она",
 "его":"её",
},
}
gettext=[
{},
{
'chips':'чипсы',
'apple':'яблоко',
'ice cream':'мороженое',
'garlic':'чеснок',
'mint':'ментол',
'rocket':'ракета',
'speed of light':'скорость света',
'decelerating':'тормозит',
'space':'космос',
'Alpha Centauri':'Альфа Центавра',
'Earth':'Земля',
'bullet':'пуля',
},
]
def translate(x):
 return gettext[LANG].get(x,x)
def translate_good(x):
 x=x.rsplit(':',1)[-1]
 return translate(x)
sims=[]
def gohome(sim):
 loc=sim.get_loc()
 if sim.at_home():
  return "Can't go home while at home"
 if sim.missions:
  return "Can't go home with pending missions"
 sim.act("%s is heading home|%s хочет домой")
 sim.missions.append(('visit',sim.home))
def icecream(sim):
 q=math.exp(quality_control['meal:ice cream'])
 sim.mass+=q
 sim.guilt+=q
def apple(sim):
 q=math.exp(quality_control['meal:apple'])
 sim.age+=q
 sim.guilt+=q
def garlic(sim):
 q=math.exp(quality_control['meal:garlic'])
 sim.mass+=q
 sim.force+=q
def mint(sim):
 q=math.exp(quality_control['meal:mint'])
 sim.mass+=q
 sim.age+=q
def chips(sim):
 q=math.exp(quality_control['meal:chips'])
 sim.mass+=q*2
def job(sim):
 if not sim.at_home():
  return "Can only go to job while well rested at home"
 job=sim.traits.get('job','')
 if not job:
  return "unemployed"
 sim.act("%s works at job|%s трудится на работе")
 sim.mass-=4
 sim.force+=1
 sim.age+=1
 sim.guilt-=1
 sim.gold+=100
 if job=='space merchant':
  sim.missions.append(('visit','Alpha Centauri'))
  #sim.missions.append(('trade until double or half',))#todo
  sim.give('bullet',5)
 elif job=='space pirate':
  if random.randrange(10)==0:
   sim.missions.append(('rob','merchants'))
   sim.give('bullet')
def movie(sim):
 sim.act("%s watches a movie|%s смотрит кино")
 sim.age-=1
def offer_food(sim):
 targets=sim.sims_nearby()
 if not targets:
  return "nobody to share with"
 sim.peer_pocket()
 offers=[k for k in sim.pocket.keys() if k in goods and k.startswith('meal:') and sim.has(k)]
 if not offers:
  return "nothing to share"
 target=random.choice(targets)
 offer=random.choice(offers)
 sim.act('%s offers %v|%s суёт %v',v=target.name+' '+translate_good(offer))
 sim.guilt-=1
 will=target.consider(get_good(offer)[0])
 greed=target.greed(offer)
 if will>-greed:
  target.act('%s takes the gift|%s берёт подарок')
  sim.take(offer)
  target.give(offer)
  target.guilt+=.5
  sim.force-=1
 else:
  target.act('%s refuses|%s вредничает')
  target.guilt*=1.01
def approach(sim,victim):
 if sim.get_loc()!=victim.get_loc():
  return "different locations"
 victim.act('%s encounters %v|%s столкнулся с %v',v=sim.name)
 victim.missions[:0]=[('survive_sim',sim)]
def threaten_blaster(sim,victim):
 if sim.get_loc()!=victim.get_loc():
  return "different locations"
 if sim.under_threat:
  return "you must defend"
 if victim.under_threat:
  return "can't double threat"
 sim.act("%s threatens %v with blaster|%s грозит %v бластером",v=victim.name)
 sim.laziness=max(0,sim.laziness-1)
 sim.guilt+=1
 victim.under_threat=sim
 victim.laziness=max(0,victim.laziness-victim.guilt)#fear
def shoot(sim,victim):
 if sim.get_loc()!=victim.get_loc():
  return "different locations"
 if sim.under_threat:
  return "you must defend"
 if victim.under_threat is not sim:
  return "you must threaten first"
 if not sim.has('bullet'):
  return 'no bullets left'
 prob=math.exp(victim.guilt-sim.guilt)
 if random.random()<prob:
  sim.act('%s shoots in %v|%s выстрелил в %v',v=victim.name)
  sim.take('bullet')
  victim.laziness=max(0,victim.laziness-10)#not sword-fight, force not used
 else:
  sim.act('%s misses!|%s мажет!')
 victim.under_threat=0
def punch(sim,victim):
 if sim.get_loc()!=victim.get_loc():
  return "different locations"
 if sim.force<0:
  return 'you are too weak'
 prob=math.exp(victim.guilt-sim.guilt)
 if random.random()<prob:
  sim.act('%s punches %v|%s бьёт %v',v=victim.name)
  victim.laziness=max(0,victim.laziness-sim.force*.1)
 else:
  sim.act('%s misses!|%s мажет!')
def desperate_leap(sim,_):
 if not sim.under_threat:
  return "no need to be desperate"
 sim.act("%s makes a desperate leap|%s делает отчаянный прыжок")
 sim.under_threat=0
 sim.guilt-=1
 sim.laziness=max(0,sim.laziness-1)
#todo balancing
actions=[
((0,0,0,0),gohome,0,0),
((1,1,1,1),job,1,0),
((0,0,5,0),movie,1,3),
((0,1,0,2),offer_food,0,1),#somehow more wants to give - less gifts
]
combat=[
threaten_blaster,
shoot,
punch,
desperate_leap
]
goods={
"meal:ice cream":((-1,0,0,-1),icecream,3,1),
"meal:apple":((0,0,-1,-1),apple,3,1),
"meal:garlic":((-1,-1,0,0),garlic,1,1),
"meal:mint":((-1,0,-1,0),mint,1,.1),
"meal:chips":((-2,0,0,0),chips,3,1),
}
def get_good(n):
 g=goods[n]
 if n in loyalty_control:
  lm=math.exp(loyalty_control[n])
  g=(tuple((c*lm for c in g[0])),)+g[1:]
 return g
def iter_goods():
 for n in goods.keys():
  yield n,get_good(n)
class Sim:
 def __init__(self,name,gender=T('it|оно'),traits=None):
  self.name=T(name)
  self.gender=gender
  if traits is None:
   self.traits={}
  else:
   self.traits=traits
  self.gold=10000
  self.under_threat=0
  self.home=('home',self.name)
  self.location=self.home
  self.pocket=defaultdict(int)
  self.missions=[]
  self.laziness=0#mass*mf+force*ff+age*af+guilt*gf>laziness makes event to happen
  #next four can be negative, that would mean sim is sick
  self.mass=0#y dimension. Grows by salt. High mass makes sim to buy what others bought, "be like everyone". Show him poverty, stat drops.
  self.force=0#x dimension. Grows by bitter. High force makes sim restless. Rest to drop.
  self.age=0#t dimension. Grows by sour. High age makes sim to prepare for "future" he imagines. If only he knew he has no future... his age would drop a bit.
  self.guilt=0#z dimension. Grows by sweets. High guilt makes sim giving things to others/trashing, and taller. Dropped by climbing mountains.
 def print_pocket(self):
  saw=0
  for name in sorted(self.pocket.keys()):
   if self.pocket[name]>0:#defauldict keeps deleted keys
    if not saw:
     self.act('%s carries:|%s несёт:')
     saw=1
    print(' '+translate_good(name)+': '+str(self.pocket[name]))
  if not saw:
   self.act('%s carries nothing|%s не несёт ничего')
 def peer_pocket(self):
  self.act('%s peers into @its pocket|%s шарит в своём кармане')
  self.print_pocket()
 def display_pocket(self):
  self.act('%s displays @its pocket|%s вывернул@ свой карман')
  self.print_pocket()
 def sims_nearby(self,filter=None):
  loc=self.get_loc()
  if filter is None:
   filter=lambda x:True
  else:
   filter=sim_filters[filter]
  return [sim for sim in sims if sim is not self
   and sim.get_loc()==loc and filter(sim)]
 def at_home(self):
  return self.get_loc()==self.home
 def has(self,name,count=1):
  return self.pocket[name]>=count
 def get_loc(self):
  return loc_aliases.get(self.location,self.location)
 def give(self,name,count=1):
  self.pocket[name]+=count
 def take(self,name,count=1):
  self.pocket[name]-=count
  if self.pocket[name]<=0:
   del self.pocket[name]
 def fmt(self,msg,caps=False,v='',g=''):
  msg=T(msg)
  msg=msg.replace('%s',self.name).replace('%v',v).replace('%g',g)
  gender=known_genders.get(self.gender,None)
  while 1:
   p=gender_pattern.match(msg)
   if p is None:
    break
   a=p.group(1)
   b=p.group(2)
   c=p.group(3)
   case=detect_case(b)
   b=b.lower()
   if gender is not None:
    b=gender.get(b,b)
   msg=a+turn_to_case(b,case)+c
  if caps:
   msg=msg.upper()
  return msg
 def act(self,*args,**kwargs):
  print(self.fmt(*args,**kwargs))
 def consider(self,vec):
  mf,ff,af,gf=vec
  return self.mass*mf+self.force*ff+self.age*af+self.guilt*gf
 def greed(self,good):
  return self.age-self.pocket[good]#tune later?
 def __call__(self):
  enemy=0
  targets=[]
  for a in [self.pocket.keys(),actions,iter_goods()]:
   for action in a:
    if type(action) is str:
     #python bug: stale inventory items with count 0 in defaultdict
     if action not in goods or not self.has(action):
      continue
     action_def=(action,)
     action=get_good(action)
    else:
     if len(action)==2:
      action=(action[1][0],(action[1][1],action[0]))+action[1][2:]
     action_def=action[1:]
     cost=action[3]
     if cost>self.gold:
      continue
    will=self.consider(action[0])
    if will>=self.laziness:
     targets.append((will,)+action_def)
  targets.sort(key=lambda x:-x[0])
  saw=0
  for target in targets:
   will_before=target[0]
   target_name=None
   target_def=target[1]
   cb=target[1]
   if type(target_def) is str:#from pocket
    if not saw:
     self.peer_pocket()
     saw=1
    target_name=target_def
    target=get_good(target_name)
    cb=target[1]
   elif type(target_def) is tuple:#fresh
    cb,target_name=target_def
   if cb(self) is None:
    self.laziness+=target[2]
    if target_name is not None and target_name.startswith('meal:'):
     self.act(T('%s eats |%s ест ')+translate_good(target_name))
     will_after=self.consider(get_good(target_name)[0])
     satisfaction=math.exp((will_before-will_after)/
      math.exp(loyalty_control[target_name]))#divide for honesty
     record('freq:'+target_name,1)#controls quality
     record('happy:'+target_name,satisfaction)#controls loyalty
    if target_name is not None and not (type(target_def) is tuple):
     if target_name.startswith('meal:'):
      self.take(target_name)
    else:
     self.gold-=target[3]
     if target_name is not None:
      extra_item_count=int(min(self.greed(target_name),self.gold/target[3]))
      if extra_item_count>0:
       if LANG==RUS:
        self.act('%s купил '+str(extra_item_count)+' '+translate_good(target_name)+' про запас')
       else:
        self.act('%s buys '+str(extra_item_count)+' more of '+translate_good(target_name))
       self.gold-=target[3]*extra_item_count
       self.give(target_name,extra_item_count)
    return
  if self.missions:
   mission=self.missions[0]
   if mission[0]=='visit':
    loc=self.get_loc()
    dest=mission[1]
    if loc==loc_aliases.get(dest,dest):
     self.missions[:1]=[]
     return
    dest_in_space=dest in space_locs or self.location in space_locs
    if dest_in_space:
     if not self.has('rocket'):
      self.act("%s rents a rocket|%s снял ракету")
      self.gold-=10
      self.give('rocket')
     if loc!='space':
      self.act("%s kicks into the space|%s взмывает в космос")
      self.location='space'
      self.under_threat=0
      return
     if self.has('decelerating'):
      self.take('decelerating')
      if not dest in space_locs:
       self.act("%s deposits a rocket|%s сдаёт ракету")
       self.take('rocket')
       self.gold+=10
     elif not self.has('speed of light'):
      self.act('%s reaches the speed of light|%s достиг скорости света')
      self.give('speed of light')
      return
     else:
      self.act('%s is decelerating|%s тормозит')
      self.give('decelerating')
      self.take('speed of light')
      return
    if dest==self.home:
     self.act('%s returns home|%s вернулся домой')
    elif dest[0]=='home':
     if LANG==RUS:
      self.act('%s прибыл в гости к '+dest[1])
     else:
      self.act('%s arrives at '+dest[1]+"'s home")
    elif type(dest)==str:
     self.act(T('%s arrives at |%s прибыл на ')+translate(dest))
    else:
     raise NotImplementedError(dest)
    self.location=dest
    self.under_threat=0
    self.missions[:1]=[]
    return
   elif mission[0]=='rob':
    victims=self.sims_nearby(mission[1])
    random.shuffle(victims)
    for victim in victims:
     if approach(self,victim):
      continue
     self.missions[0]=('rob_sim',victim)
     return
    else:
     self.missions[:1]=[]
   elif mission[0] in ('rob_sim','survive_sim'):
    enemy=mission[1]
  if enemy:
   random.shuffle(combat)
   for action in combat:
    if action(self,enemy) is None:
     break
   else:
    enemy.win(self)
    return
   if enemy.laziness<=0:
    self.win(enemy)
   if self.laziness<=0:
    enemy.win(self)
   return
  self.act("%s relaxes|%s отдыхает")
  if self.laziness>0:
   f=math.exp(self.force*.15)#just self.force is too much random
   self.laziness=max(self.laziness-f,0)
  self.force*=.95
 def win(self,loser):
  self.act('%s wins over %v!!!|%s одолел %v!!!',v=loser.name,caps=True)
  if loser.under_threat is self:
   loser.under_threat=0
  if self.under_threat is loser:
   self.under_threat=0
  loser.missions[:1]=[]
  if 'rob' in self.missions[0]:
   loot=max(loser.gold>>1,0)
   loser.act('%s gave %v %g gold|%s отдал %v %g золота',v=self.name,g=str(loot))
   loser.gold-=loot
   self.gold+=loot
   for k,v in loser.pocket.items():
    if k in goods and v>0:
     loser.act('%s gave %v |%s отдал %v ',v=self.name,g=str(v)+' '+translate_good(k))
     loser.take(k,v)
     self.give(k,v)
  self.missions[:1]=[]
  wins[self.name]+=1
  losses[loser.name]+=1
sims.append(Sim('Peter|Питер',gender='he',traits={'job':'space merchant'}))
sims.append(Sim('Zorloc|Зорлок',gender='he',traits={'job':'space pirate'}))
#while 1:
for i in range(4096):
 y,d=get_year()
 h=date(y+1,1,1)+timedelta(days=d)
 print(h.strftime("%m/%d/%04Y"))
 random.shuffle(sims)
 for sim in sims:
  sim()
  #time.sleep(1)
 research_turn()
pprint(research)
print('quality:')
pprint(quality_control)
print('loyalty:')
pprint(loyalty_control)
print('wins:')
pprint(wins)
print('losses:')
pprint(losses)
