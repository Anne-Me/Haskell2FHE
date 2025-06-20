module Example.Project (topEntity, pir) where

import Clash.Prelude

pir :: Unsigned 32 -> Vec 100 (Unsigned 32) -> Bool
pir x database = or (map (x == ) database)

-- | 'topEntity' is Clash's equivalent of 'main' in other programming
-- languages. Clash will look for it when compiling 'Example.Project'
-- and translate it to HDL. While polymorphism can be used freely in
-- Clash projects, a 'topEntity' must be monomorphic and must use non-
-- recursive types. Or, to put it hand-wavily, a 'topEntity' must be
-- translatable to a static number of wires.
topEntity :: Unsigned 32 -> Vec 100 (Unsigned 32) -> Bool
topEntity = pir
