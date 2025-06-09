module Example.Project (topEntity, addtwo) where

import Clash.Prelude

addtwo :: Unsigned 64 -> Unsigned 64 -> Unsigned 64
addtwo a b = a + b

-- | 'topEntity' is Clash's equivalent of 'main' in other programming
-- languages. Clash will look for it when compiling 'Example.Project'
-- and translate it to HDL. While polymorphism can be used freely in
-- Clash projects, a 'topEntity' must be monomorphic and must use non-
-- recursive types. Or, to put it hand-wavily, a 'topEntity' must be
-- translatable to a static number of wires.
topEntity :: Unsigned 64 -> Unsigned 64 -> Unsigned 64
topEntity = addtwo 
