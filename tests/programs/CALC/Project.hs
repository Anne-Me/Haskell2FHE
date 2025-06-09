module Example.Project (topEntity, calc) where

import Clash.Prelude

-- calc some stuff
calc :: Signed 8 -> Signed 8 -> Signed 8 -> Signed 8
calc a b c = a + b * c - a  + ((b + c) * 3) - (c*2)

-- | 'topEntity' is Clash's equivalent of 'main' in other programming
-- languages. Clash will look for it when compiling 'Example.Project'
-- and translate it to HDL. While polymorphism can be used freely in
-- Clash projects, a 'topEntity' must be monomorphic and must use non-
-- recursive types. Or, to put it hand-wavily, a 'topEntity' must be
-- translatable to a static number of wires.
topEntity :: Signed 8 -> Signed 8 -> Signed 8 -> Signed 8
topEntity = calc
